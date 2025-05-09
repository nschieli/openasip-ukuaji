/*
    Copyright (c) 2002-2009 Tampere University.

    This file is part of TTA-Based Codesign Environment (TCE).

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
 */
/**
 * @file CodeSymElement.hh
 *
 * Declaration of CodeSymElement class.
 *
 * @author Mikael Lepistö 2004 (tmlepist-no.spam-cs.tut.fi)
 *
 * @note rating: yellow
 */

#ifndef TTA_CODE_SYM_ELEMENT_HH
#define TTA_CODE_SYM_ELEMENT_HH

#include "SymbolElement.hh"

namespace TPEF {

    class InstructionElement;

/**
 * Code symbol.
 */
class CodeSymElement : public SymbolElement {
 public:
    CodeSymElement();
    virtual ~CodeSymElement();

    virtual SymbolType type() const;

    InstructionElement* reference() const;
    void setReference(InstructionElement* aReference);
    void setReference(const ReferenceManager::SafePointer* aReference);

    Word size() const;
    void setSize(Word aSize);

 private:
    /// Referred object.
    const ReferenceManager::SafePointer* reference_;
    /// Size of referred object in MAUs.
    Word size_;
};
}

#endif
