/* Cycript - Optimizing JavaScript Compiler/Runtime
 * Copyright (C) 2009-2013  Jay Freeman (saurik)
*/

/* GNU General Public License, Version 3 {{{ */
/*
 * Cycript is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cycript is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cycript.  If not, see <http://www.gnu.org/licenses/>.
**/
/* }}} */

#ifndef CYCRIPT_OBJECTIVEC_SYNTAX_HPP
#define CYCRIPT_OBJECTIVEC_SYNTAX_HPP

#include "Parser.hpp"

struct CYTypeModifier :
    CYNext<CYTypeModifier>
{
    CYTypeModifier(CYTypeModifier *next) :
        CYNext<CYTypeModifier>(next)
    {
    }

    virtual CYExpression *Replace(CYContext &context) = 0;
};

struct CYTypeArrayOf :
    CYTypeModifier
{
    CYExpression *size_;

    CYTypeArrayOf(CYExpression *size, CYTypeModifier *next = NULL) :
        CYTypeModifier(next),
        size_(size)
    {
    }

    CYPrecedence(2)

    virtual CYExpression *Replace(CYContext &context);
};

struct CYTypeConstant :
    CYTypeModifier
{
    CYTypeConstant(CYTypeModifier *next = NULL) :
        CYTypeModifier(next)
    {
    }

    CYPrecedence(3)

    virtual CYExpression *Replace(CYContext &context);
};

struct CYTypePointerTo :
    CYTypeModifier
{
    CYTypePointerTo(CYTypeModifier *next = NULL) :
        CYTypeModifier(next)
    {
    }

    CYPrecedence(3)

    virtual CYExpression *Replace(CYContext &context);
};

struct CYTypeVariable :
    CYTypeModifier
{
    CYExpression *expression_;

    CYTypeVariable(CYExpression *expression) :
        CYTypeModifier(NULL),
        expression_(expression)
    {
    }

    CYPrecedence(1)

    virtual CYExpression *Replace(CYContext &context);
};

struct CYTypedIdentifier :
    CYNext<CYTypedIdentifier>
{
    CYIdentifier *identifier_;
    CYTypeModifier *type_;

    CYTypedIdentifier(CYIdentifier *identifier) :
        identifier_(identifier),
        type_(NULL)
    {
    }
};

struct CYTypedParameter :
    CYNext<CYTypedParameter>
{
    CYTypedIdentifier *typed_;

    CYTypedParameter(CYTypedIdentifier *typed, CYTypedParameter *next) :
        CYNext<CYTypedParameter>(next),
        typed_(typed)
    {
    }

    CYFunctionParameter *Parameters(CYContext &context);
    CYExpression *TypeSignature(CYContext &context, CYExpression *prefix);
};

struct CYObjCBlock :
    CYExpression
{
    CYTypeModifier *type_;
    CYTypedParameter *parameters_;
    CYStatement *statements_;

    CYObjCBlock(CYTypeModifier *type, CYTypedParameter *parameters, CYStatement *statements) :
        type_(type),
        parameters_(parameters),
        statements_(statements)
    {
    }

    CYPrecedence(1)

    virtual CYExpression *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYEncodedType :
    CYExpression
{
    CYTypeModifier *type_;

    CYEncodedType(CYTypeModifier *type) :
        type_(type)
    {
    }

    CYPrecedence(1)

    virtual CYExpression *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYBox :
    CYExpression
{
    CYExpression *value_;

    CYBox(CYExpression *value) :
        value_(value)
    {
    }

    CYPrecedence(1)

    virtual CYExpression *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYSelectorPart :
    CYNext<CYSelectorPart>,
    CYThing
{
    CYWord *name_;
    bool value_;

    CYSelectorPart(CYWord *name, bool value, CYSelectorPart *next = NULL) :
        CYNext<CYSelectorPart>(next),
        name_(name),
        value_(value)
    {
    }

    CYString *Replace(CYContext &context);
    virtual void Output(CYOutput &out) const;
};

struct CYSelector :
    CYLiteral
{
    CYSelectorPart *name_;

    CYSelector(CYSelectorPart *name) :
        name_(name)
    {
    }

    CYPrecedence(1)

    virtual CYExpression *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYField :
    CYNext<CYField>
{
    CYExpression *type_;
    CYIdentifier *name_;

    CYField(CYExpression *type, CYIdentifier *name, CYField *next = NULL) :
        CYNext<CYField>(next),
        type_(type),
        name_(name)
    {
    }

    CYStatement *Replace(CYContext &context) const;
    void Output(CYOutput &out) const;
};

struct CYMessageParameter :
    CYNext<CYMessageParameter>
{
    CYWord *tag_;
    CYExpression *type_;
    CYIdentifier *name_;

    CYMessageParameter(CYWord *tag, CYExpression *type, CYIdentifier *name) :
        tag_(tag),
        type_(type),
        name_(name)
    {
    }

    CYFunctionParameter *Parameters(CYContext &context) const;
    CYSelector *Selector(CYContext &context) const;
    CYSelectorPart *SelectorPart(CYContext &context) const;
    CYExpression *TypeSignature(CYContext &context) const;
};

struct CYMessage :
    CYNext<CYMessage>
{
    bool instance_;
    CYExpression *type_;
    CYMessageParameter *parameters_;
    CYBlock code_;

    CYMessage(bool instance, CYExpression *type, CYMessageParameter *parameter, CYStatement *statements) :
        instance_(instance),
        type_(type),
        parameters_(parameter),
        code_(statements)
    {
    }

    CYStatement *Replace(CYContext &context, bool replace) const;
    void Output(CYOutput &out, bool replace) const;

    CYExpression *TypeSignature(CYContext &context) const;
};

struct CYProtocol :
    CYNext<CYProtocol>,
    CYThing
{
    CYExpression *name_;

    CYProtocol(CYExpression *name, CYProtocol *next = NULL) :
        CYNext<CYProtocol>(next),
        name_(name)
    {
    }

    CYStatement *Replace(CYContext &context) const;
    void Output(CYOutput &out) const;
};

struct CYImport :
    CYStatement
{
    virtual CYStatement *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYClass {
    CYClassName *name_;
    CYExpression *super_;
    CYProtocol *protocols_;
    CYField *fields_;
    CYMessage *messages_;

    CYClass(CYClassName *name, CYExpression *super, CYProtocol *protocols, CYField *fields, CYMessage *messages) :
        name_(name),
        super_(super),
        protocols_(protocols),
        fields_(fields),
        messages_(messages)
    {
    }

    virtual ~CYClass() {
    }

    CYExpression *Replace_(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYClassExpression :
    CYClass,
    CYExpression
{
    CYClassExpression(CYClassName *name, CYExpression *super, CYProtocol *protocols, CYField *fields, CYMessage *messages) :
        CYClass(name, super, protocols, fields, messages)
    {
    }

    CYPrecedence(0)

    virtual CYExpression *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYClassStatement :
    CYClass,
    CYStatement
{
    CYClassStatement(CYClassName *name, CYExpression *super, CYProtocol *protocols, CYField *fields, CYMessage *messages) :
        CYClass(name, super, protocols, fields, messages)
    {
    }

    virtual CYStatement *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYCategory :
    CYStatement
{
    CYClassName *name_;
    CYMessage *messages_;

    CYCategory(CYClassName *name, CYMessage *messages) :
        name_(name),
        messages_(messages)
    {
    }

    virtual CYStatement *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYSend :
    CYExpression
{
    CYArgument *arguments_;

    CYSend(CYArgument *arguments) :
        arguments_(arguments)
    {
    }

    CYPrecedence(0)

    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYSendDirect :
    CYSend
{
    CYExpression *self_;

    CYSendDirect(CYExpression *self, CYArgument *arguments) :
        CYSend(arguments),
        self_(self)
    {
    }

    virtual CYExpression *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

struct CYSendSuper :
    CYSend
{
    CYSendSuper(CYArgument *arguments) :
        CYSend(arguments)
    {
    }

    virtual CYExpression *Replace(CYContext &context);
    virtual void Output(CYOutput &out, CYFlags flags) const;
};

#endif/*CYCRIPT_OBJECTIVEC_SYNTAX_HPP*/
