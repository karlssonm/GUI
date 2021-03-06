/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_XmlDocument.h"
#include "../io/streams/juce_FileInputSource.h"
#include "../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
XmlDocument::XmlDocument (const String& documentText)
    : originalText (documentText),
      ignoreEmptyTextElements (true)
{
}

XmlDocument::XmlDocument (const File& file)
    : ignoreEmptyTextElements (true),
      inputSource (new FileInputSource (file))
{
}

XmlDocument::~XmlDocument()
{
}

XmlElement* XmlDocument::parse (const File& file)
{
    XmlDocument doc (file);
    return doc.getDocumentElement();
}

XmlElement* XmlDocument::parse (const String& xmlData)
{
    XmlDocument doc (xmlData);
    return doc.getDocumentElement();
}

void XmlDocument::setInputSource (InputSource* const newSource) throw()
{
    inputSource = newSource;
}

void XmlDocument::setEmptyTextElementsIgnored (const bool shouldBeIgnored) throw()
{
    ignoreEmptyTextElements = shouldBeIgnored;
}

namespace XmlIdentifierChars
{
    bool isIdentifierCharSlow (const juce_wchar c) throw()
    {
        return CharacterFunctions::isLetterOrDigit (c)
                 || c == '_' || c == '-' || c == ':' || c == '.';
    }

    bool isIdentifierChar (const juce_wchar c) throw()
    {
        static const uint32 legalChars[] = { 0, 0x7ff6000, 0x87fffffe, 0x7fffffe, 0 };

        return (c < numElementsInArray (legalChars) * 32) ? ((legalChars [c >> 5] & (1 << (c & 31))) != 0)
                                                          : isIdentifierCharSlow (c);
    }

    /*static void generateIdentifierCharConstants()
    {
        uint32 n[8];
        zerostruct (n);
        for (int i = 0; i < 256; ++i)
            if (isIdentifierCharSlow (i))
                n[i >> 5] |= (1 << (i & 31));

        String s;
        for (int i = 0; i < 8; ++i)
            s << "0x" << String::toHexString ((int) n[i]) << ", ";

        DBG (s);
    }*/
}

XmlElement* XmlDocument::getDocumentElement (const bool onlyReadOuterDocumentElement)
{
    String textToParse (originalText);

    if (textToParse.isEmpty() && inputSource != 0)
    {
        ScopedPointer <InputStream> in (inputSource->createInputStream());

        if (in != 0)
        {
            MemoryOutputStream data;
            data.writeFromInputStream (*in, onlyReadOuterDocumentElement ? 8192 : -1);
            textToParse = data.toString();

            if (! onlyReadOuterDocumentElement)
                originalText = textToParse;
        }
    }

    input = textToParse;
    lastError = String::empty;
    errorOccurred = false;
    outOfData = false;
    needToLoadDTD = true;

    if (textToParse.isEmpty())
    {
        lastError = "not enough input";
    }
    else
    {
        skipHeader();

        if (input != 0)
        {
            ScopedPointer <XmlElement> result (readNextElement (! onlyReadOuterDocumentElement));

            if (! errorOccurred)
                return result.release();
        }
        else
        {
            lastError = "incorrect xml header";
        }
    }

    return 0;
}

const String& XmlDocument::getLastParseError() const throw()
{
    return lastError;
}

void XmlDocument::setLastError (const String& desc, const bool carryOn)
{
    lastError = desc;
    errorOccurred = ! carryOn;
}

const String XmlDocument::getFileContents (const String& filename) const
{
    if (inputSource != 0)
    {
        const ScopedPointer <InputStream> in (inputSource->createInputStreamFor (filename.trim().unquoted()));

        if (in != 0)
            return in->readEntireStreamAsString();
    }

    return String::empty;
}

juce_wchar XmlDocument::readNextChar() throw()
{
    if (*input != 0)
        return *input++;

    outOfData = true;
    return 0;
}

int XmlDocument::findNextTokenLength() throw()
{
    int len = 0;
    juce_wchar c = *input;

    while (XmlIdentifierChars::isIdentifierChar (c))
        c = input [++len];

    return len;
}

void XmlDocument::skipHeader()
{
    const juce_wchar* const found = CharacterFunctions::find (input, JUCE_T("<?xml"));

    if (found != 0)
    {
        input = found;
        input = CharacterFunctions::find (input, JUCE_T("?>"));

        if (input == 0)
            return;

       #if JUCE_DEBUG
        const String header (found, input - found);
        const String encoding (header.fromFirstOccurrenceOf ("encoding", false, true)
                                     .fromFirstOccurrenceOf ("=", false, false)
                                     .fromFirstOccurrenceOf ("\"", false, false)
                                     .upToFirstOccurrenceOf ("\"", false, false).trim());

        /* If you load an XML document with a non-UTF encoding type, it may have been
           loaded wrongly.. Since all the files are read via the normal juce file streams,
           they're treated as UTF-8, so by the time it gets to the parser, the encoding will
           have been lost. Best plan is to stick to utf-8 or if you have specific files to
           read, use your own code to convert them to a unicode String, and pass that to the
           XML parser.
        */
        jassert (encoding.isEmpty() || encoding.startsWithIgnoreCase ("utf-"));
       #endif

        input += 2;
    }

    skipNextWhiteSpace();
    const juce_wchar* docType = CharacterFunctions::find (input, JUCE_T("<!DOCTYPE"));

    if (docType == 0)
        return;

    input = docType + 9;

    int n = 1;

    while (n > 0)
    {
        const juce_wchar c = readNextChar();

        if (outOfData)
            return;

        if (c == '<')
            ++n;
        else if (c == '>')
            --n;
    }

    docType += 9;
    dtdText = String (docType, (int) (input - (docType + 1))).trim();
}

void XmlDocument::skipNextWhiteSpace()
{
    for (;;)
    {
        juce_wchar c = *input;

        while (CharacterFunctions::isWhitespace (c))
            c = *++input;

        if (c == 0)
        {
            outOfData = true;
            break;
        }
        else if (c == '<')
        {
            if (input[1] == '!'
                 && input[2] == '-'
                 && input[3] == '-')
            {
                const juce_wchar* const closeComment = CharacterFunctions::find (input, JUCE_T("-->"));

                if (closeComment == 0)
                {
                    outOfData = true;
                    break;
                }

                input = closeComment + 3;
                continue;
            }
            else if (input[1] == '?')
            {
                const juce_wchar* const closeBracket = CharacterFunctions::find (input, JUCE_T("?>"));

                if (closeBracket == 0)
                {
                    outOfData = true;
                    break;
                }

                input = closeBracket + 2;
                continue;
            }
        }

        break;
    }
}

void XmlDocument::readQuotedString (String& result)
{
    const juce_wchar quote = readNextChar();

    while (! outOfData)
    {
        const juce_wchar c = readNextChar();

        if (c == quote)
            break;

        if (c == '&')
        {
            --input;
            readEntity (result);
        }
        else
        {
            --input;
            const juce_wchar* const start = input;

            for (;;)
            {
                const juce_wchar character = *input;

                if (character == quote)
                {
                    result.append (start, (int) (input - start));
                    ++input;

                    return;
                }
                else if (character == '&')
                {
                    result.append (start, (int) (input - start));
                    break;
                }
                else if (character == 0)
                {
                    outOfData = true;
                    setLastError ("unmatched quotes", false);
                    break;
                }

                ++input;
            }
        }
    }
}

XmlElement* XmlDocument::readNextElement (const bool alsoParseSubElements)
{
    XmlElement* node = 0;

    skipNextWhiteSpace();
    if (outOfData)
        return 0;

    input = CharacterFunctions::find (input, JUCE_T("<"));

    if (input != 0)
    {
        ++input;
        int tagLen = findNextTokenLength();

        if (tagLen == 0)
        {
            // no tag name - but allow for a gap after the '<' before giving an error
            skipNextWhiteSpace();
            tagLen = findNextTokenLength();

            if (tagLen == 0)
            {
                setLastError ("tag name missing", false);
                return node;
            }
        }

        node = new XmlElement (String (input, tagLen));
        input += tagLen;
        XmlElement::XmlAttributeNode* lastAttribute = 0;

        // look for attributes
        for (;;)
        {
            skipNextWhiteSpace();

            const juce_wchar c = *input;

            // empty tag..
            if (c == '/' && input[1] == '>')
            {
                input += 2;
                break;
            }

            // parse the guts of the element..
            if (c == '>')
            {
                ++input;

                if (alsoParseSubElements)
                    readChildElements (node);

                break;
            }

            // get an attribute..
            if (XmlIdentifierChars::isIdentifierChar (c))
            {
                const int attNameLen = findNextTokenLength();

                if (attNameLen > 0)
                {
                    const juce_wchar* attNameStart = input;
                    input += attNameLen;

                    skipNextWhiteSpace();

                    if (readNextChar() == '=')
                    {
                        skipNextWhiteSpace();

                        const juce_wchar nextChar = *input;

                        if (nextChar == '"' || nextChar == '\'')
                        {
                            XmlElement::XmlAttributeNode* const newAtt
                                = new XmlElement::XmlAttributeNode (String (attNameStart, attNameLen),
                                                                    String::empty);

                            readQuotedString (newAtt->value);

                            if (lastAttribute == 0)
                                node->attributes = newAtt;
                            else
                                lastAttribute->next = newAtt;

                            lastAttribute = newAtt;

                            continue;
                        }
                    }
                }
            }
            else
            {
                if (! outOfData)
                    setLastError ("illegal character found in " + node->getTagName() + ": '" + c + "'", false);
            }

            break;
        }
    }

    return node;
}

void XmlDocument::readChildElements (XmlElement* parent)
{
    XmlElement* lastChildNode = 0;

    for (;;)
    {
        const juce_wchar* const preWhitespaceInput = input;
        skipNextWhiteSpace();

        if (outOfData)
        {
            setLastError ("unmatched tags", false);
            break;
        }

        if (*input == '<')
        {
            if (input[1] == '/')
            {
                // our close tag..
                input = CharacterFunctions::find (input, JUCE_T(">"));
                ++input;
                break;
            }
            else if (input[1] == '!'
                  && input[2] == '['
                  && input[3] == 'C'
                  && input[4] == 'D'
                  && input[5] == 'A'
                  && input[6] == 'T'
                  && input[7] == 'A'
                  && input[8] == '[')
            {
                input += 9;
                const juce_wchar* const inputStart = input;

                int len = 0;

                for (;;)
                {
                    if (*input == 0)
                    {
                        setLastError ("unterminated CDATA section", false);
                        outOfData = true;
                        break;
                    }
                    else if (input[0] == ']'
                              && input[1] == ']'
                              && input[2] == '>')
                    {
                        input += 3;
                        break;
                    }

                    ++input;
                    ++len;
                }

                XmlElement* const e = XmlElement::createTextElement (String (inputStart, len));

                if (lastChildNode != 0)
                    lastChildNode->nextElement = e;
                else
                    parent->addChildElement (e);

                lastChildNode = e;
            }
            else
            {
                // this is some other element, so parse and add it..
                XmlElement* const n = readNextElement (true);

                if (n != 0)
                {
                    if (lastChildNode == 0)
                        parent->addChildElement (n);
                    else
                        lastChildNode->nextElement = n;

                    lastChildNode = n;
                }
                else
                {
                    return;
                }
            }
        }
        else  // must be a character block
        {
            input = preWhitespaceInput; // roll back to include the leading whitespace
            String textElementContent;

            for (;;)
            {
                const juce_wchar c = *input;

                if (c == '<')
                    break;

                if (c == 0)
                {
                    setLastError ("unmatched tags", false);
                    outOfData = true;
                    return;
                }

                if (c == '&')
                {
                    String entity;
                    readEntity (entity);

                    if (entity.startsWithChar ('<') && entity [1] != 0)
                    {
                        const juce_wchar* const oldInput = input;
                        const bool oldOutOfData = outOfData;

                        input = entity;
                        outOfData = false;

                        for (;;)
                        {
                            XmlElement* const n = readNextElement (true);

                            if (n == 0)
                                break;

                            if (lastChildNode == 0)
                                parent->addChildElement (n);
                            else
                                lastChildNode->nextElement = n;

                            lastChildNode = n;
                        }

                        input = oldInput;
                        outOfData = oldOutOfData;
                    }
                    else
                    {
                        textElementContent += entity;
                    }
                }
                else
                {
                    const juce_wchar* start = input;
                    int len = 0;

                    for (;;)
                    {
                        const juce_wchar nextChar = *input;

                        if (nextChar == '<' || nextChar == '&')
                        {
                            break;
                        }
                        else if (nextChar == 0)
                        {
                            setLastError ("unmatched tags", false);
                            outOfData = true;
                            return;
                        }

                        ++input;
                        ++len;
                    }

                    textElementContent.append (start, len);
                }
            }

            if ((! ignoreEmptyTextElements) || textElementContent.containsNonWhitespaceChars())
            {
                XmlElement* const textElement = XmlElement::createTextElement (textElementContent);

                if (lastChildNode != 0)
                    lastChildNode->nextElement = textElement;
                else
                    parent->addChildElement (textElement);

                lastChildNode = textElement;
            }
        }
    }
}

void XmlDocument::readEntity (String& result)
{
    // skip over the ampersand
    ++input;

    if (CharacterFunctions::compareIgnoreCase (input, JUCE_T("amp;"), 4) == 0)
    {
        input += 4;
        result += '&';
    }
    else if (CharacterFunctions::compareIgnoreCase (input, JUCE_T("quot;"), 5) == 0)
    {
        input += 5;
        result += '"';
    }
    else if (CharacterFunctions::compareIgnoreCase (input, JUCE_T("apos;"), 5) == 0)
    {
        input += 5;
        result += '\'';
    }
    else if (CharacterFunctions::compareIgnoreCase (input, JUCE_T("lt;"), 3) == 0)
    {
        input += 3;
        result += '<';
    }
    else if (CharacterFunctions::compareIgnoreCase (input, JUCE_T("gt;"), 3) == 0)
    {
        input += 3;
        result += '>';
    }
    else if (*input == '#')
    {
        int charCode = 0;
        ++input;

        if (*input == 'x' || *input == 'X')
        {
            ++input;
            int numChars = 0;

            while (input[0] != ';')
            {
                const int hexValue = CharacterFunctions::getHexDigitValue (input[0]);

                if (hexValue < 0 || ++numChars > 8)
                {
                    setLastError ("illegal escape sequence", true);
                    break;
                }

                charCode = (charCode << 4) | hexValue;
                ++input;
            }

            ++input;
        }
        else if (input[0] >= '0' && input[0] <= '9')
        {
            int numChars = 0;

            while (input[0] != ';')
            {
                if (++numChars > 12)
                {
                    setLastError ("illegal escape sequence", true);
                    break;
                }

                charCode = charCode * 10 + (input[0] - '0');
                ++input;
            }

            ++input;
        }
        else
        {
            setLastError ("illegal escape sequence", true);
            result += '&';
            return;
        }

        result << (juce_wchar) charCode;
    }
    else
    {
        const juce_wchar* const entityNameStart = input;
        const juce_wchar* const closingSemiColon = CharacterFunctions::find (input, JUCE_T(";"));

        if (closingSemiColon == 0)
        {
            outOfData = true;
            result += '&';
        }
        else
        {
            input = closingSemiColon + 1;

            result += expandExternalEntity (String (entityNameStart,
                                                    (int) (closingSemiColon - entityNameStart)));
        }
    }
}

const String XmlDocument::expandEntity (const String& ent)
{
    if (ent.equalsIgnoreCase ("amp"))   return String::charToString ('&');
    if (ent.equalsIgnoreCase ("quot"))  return String::charToString ('"');
    if (ent.equalsIgnoreCase ("apos"))  return String::charToString ('\'');
    if (ent.equalsIgnoreCase ("lt"))    return String::charToString ('<');
    if (ent.equalsIgnoreCase ("gt"))    return String::charToString ('>');

    if (ent[0] == '#')
    {
        if (ent[1] == 'x' || ent[1] == 'X')
            return String::charToString (static_cast <juce_wchar> (ent.substring (2).getHexValue32()));

        if (ent[1] >= '0' && ent[1] <= '9')
            return String::charToString (static_cast <juce_wchar> (ent.substring (1).getIntValue()));

        setLastError ("illegal escape sequence", false);
        return String::charToString ('&');
    }

    return expandExternalEntity (ent);
}

const String XmlDocument::expandExternalEntity (const String& entity)
{
    if (needToLoadDTD)
    {
        if (dtdText.isNotEmpty())
        {
            dtdText = dtdText.trimCharactersAtEnd (">");
            tokenisedDTD.addTokens (dtdText, true);

            if (tokenisedDTD [tokenisedDTD.size() - 2].equalsIgnoreCase ("system")
                 && tokenisedDTD [tokenisedDTD.size() - 1].isQuotedString())
            {
                const String fn (tokenisedDTD [tokenisedDTD.size() - 1]);

                tokenisedDTD.clear();
                tokenisedDTD.addTokens (getFileContents (fn), true);
            }
            else
            {
                tokenisedDTD.clear();
                const int openBracket = dtdText.indexOfChar ('[');

                if (openBracket > 0)
                {
                    const int closeBracket = dtdText.lastIndexOfChar (']');

                    if (closeBracket > openBracket)
                        tokenisedDTD.addTokens (dtdText.substring (openBracket + 1,
                                                                   closeBracket), true);
                }
            }

            for (int i = tokenisedDTD.size(); --i >= 0;)
            {
                if (tokenisedDTD[i].startsWithChar ('%')
                     && tokenisedDTD[i].endsWithChar (';'))
                {
                    const String parsed (getParameterEntity (tokenisedDTD[i].substring (1, tokenisedDTD[i].length() - 1)));
                    StringArray newToks;
                    newToks.addTokens (parsed, true);

                    tokenisedDTD.remove (i);

                    for (int j = newToks.size(); --j >= 0;)
                        tokenisedDTD.insert (i, newToks[j]);
                }
            }
        }

        needToLoadDTD = false;
    }

    for (int i = 0; i < tokenisedDTD.size(); ++i)
    {
        if (tokenisedDTD[i] == entity)
        {
            if (tokenisedDTD[i - 1].equalsIgnoreCase ("<!entity"))
            {
                String ent (tokenisedDTD [i + 1].trimCharactersAtEnd (">").trim().unquoted());

                // check for sub-entities..
                int ampersand = ent.indexOfChar ('&');

                while (ampersand >= 0)
                {
                    const int semiColon = ent.indexOf (i + 1, ";");

                    if (semiColon < 0)
                    {
                        setLastError ("entity without terminating semi-colon", false);
                        break;
                    }

                    const String resolved (expandEntity (ent.substring (i + 1, semiColon)));

                    ent = ent.substring (0, ampersand)
                           + resolved
                           + ent.substring (semiColon + 1);

                    ampersand = ent.indexOfChar (semiColon + 1, '&');
                }

                return ent;
            }
        }
    }

    setLastError ("unknown entity", true);

    return entity;
}

const String XmlDocument::getParameterEntity (const String& entity)
{
    for (int i = 0; i < tokenisedDTD.size(); ++i)
    {
        if (tokenisedDTD[i] == entity)
        {
            if (tokenisedDTD [i - 1] == "%"
                && tokenisedDTD [i - 2].equalsIgnoreCase ("<!entity"))
            {
                const String ent (tokenisedDTD [i + 1].trimCharactersAtEnd (">"));

                if (ent.equalsIgnoreCase ("system"))
                    return getFileContents (tokenisedDTD [i + 2].trimCharactersAtEnd (">"));
                else
                    return ent.trim().unquoted();
            }
        }
    }

    return entity;
}


END_JUCE_NAMESPACE
