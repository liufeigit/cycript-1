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

#include "Replace.hpp"
#include "ObjectiveC/Syntax.hpp"

#include <sstream>

static CYExpression *MessageType(CYContext &context, CYExpression *type, CYMessageParameter *next, CYExpression *extra = NULL) {
    if (type == NULL)
        return NULL;

    CYExpression *left($C0($M(type, $S("toString"))));
    if (extra != NULL)
        left = $ CYAdd(left, extra);

    if (next == NULL || next->name_ == NULL)
        return left;

    CYExpression *right(next->TypeSignature(context));
    if (right == NULL)
        return NULL;

    return $ CYAdd(left, right);
}

CYStatement *CYCategory::Replace(CYContext &context) {
    CYVariable *cyc($V("$cyc")), *cys($V("$cys"));

    return $E($C1($F(NULL, $P6($L("$cys"), $L("$cyp"), $L("$cyc"), $L("$cyn"), $L("$cyt"), $L("$cym")), $$->*
        $E($ CYAssign($V("$cyp"), $C1($V("object_getClass"), cys)))->*
        $E($ CYAssign(cyc, cys))->*
        $E($ CYAssign($V("$cym"), $C1($V("object_getClass"), cyc)))->*
        messages_->Replace(context, true)
    ), name_->ClassName(context, true)));
}

CYExpression *CYClass::Replace_(CYContext &context) {
    CYVariable *cyc($V("$cyc")), *cys($V("$cys"));

    CYExpression *name(name_ != NULL ? name_->ClassName(context, false) : $C1($V("$cyq"), $S("CY$")));

    return $C1($F(NULL, $P6($L("$cys"), $L("$cyp"), $L("$cyc"), $L("$cyn"), $L("$cyt"), $L("$cym")), $$->*
        $E($ CYAssign($V("$cyp"), $C1($V("object_getClass"), cys)))->*
        $E($ CYAssign(cyc, $C3($V("objc_allocateClassPair"), cys, name, $D(0))))->*
        $E($ CYAssign($V("$cym"), $C1($V("object_getClass"), cyc)))->*
        protocols_->Replace(context)->*
        fields_->Replace(context)->*
        messages_->Replace(context, false)->*
        $E($C1($V("objc_registerClassPair"), cyc))->*
        $ CYReturn(cyc)
    ), super_ == NULL ? $ CYNull() : super_);
}

CYExpression *CYClassExpression::Replace(CYContext &context) {
    return Replace_(context);
}

CYStatement *CYClassStatement::Replace(CYContext &context) {
    return $E(Replace_(context));
}

CYExpression *CYTypeArrayOf::Replace(CYContext &context) {
    return $ CYCall($ CYDirectMember(next_->Replace(context), $ CYString("arrayOf")), $ CYArgument(size_));
}

CYExpression *CYTypeConstant::Replace(CYContext &context) {
    return $ CYCall($ CYDirectMember(next_->Replace(context), $ CYString("constant")));
}

CYExpression *CYTypePointerTo::Replace(CYContext &context) {
    return $ CYCall($ CYDirectMember(next_->Replace(context), $ CYString("pointerTo")));
}

CYExpression *CYTypeVariable::Replace(CYContext &context) {
    return expression_;
}

CYExpression *CYEncodedType::Replace(CYContext &context) {
    return type_->Replace(context);
}

CYStatement *CYField::Replace(CYContext &context) const { $T(NULL)
    CYVariable *cyn($V("$cyn"));
    CYVariable *cyt($V("$cyt"));

    CYExpression *type($C0($M(type_, $S("toString"))));

    return $ CYBlock($$->*
        next_->Replace(context)->*
        $E($ CYAssign(cyt, type))->*
        $E($ CYAssign(cyn, $N1($V("Type"), cyt)))->*
        $E($C5($V("class_addIvar"), $V("$cyc"), $S(name_->Word()), $M(cyn, $S("size")), $M(cyn, $S("alignment")), cyt))
    );
}

CYStatement *CYImport::Replace(CYContext &context) {
    return this;
}

CYStatement *CYMessage::Replace(CYContext &context, bool replace) const { $T(NULL)
    CYVariable *cyn($V("$cyn"));
    CYVariable *cyt($V("$cyt"));
    CYVariable *self($V("self"));
    CYVariable *_class($V(instance_ ? "$cys" : "$cyp"));

    CYExpression *type(TypeSignature(context) ?: $C1($M(cyn, $S("type")), _class));

    return $ CYBlock($$->*
        next_->Replace(context, replace)->*
        $E($ CYAssign(cyn, parameters_->Selector(context)))->*
        $E($ CYAssign(cyt, type))->*
        $E($C4($V(replace ? "class_replaceMethod" : "class_addMethod"),
            $V(instance_ ? "$cyc" : "$cym"),
            cyn,
            $N2($V("Functor"), $F(NULL, $P2($L("self"), $L("_cmd"), parameters_->Parameters(context)), $$->*
                $ CYVar($L1($L("$cyr", $N2($V("Super"), self, _class))))->*
                $ CYReturn($C1($M($F(NULL, NULL, code_), $S("call")), self))
            ), cyt),
            cyt
        ))
    );
}

CYExpression *CYMessage::TypeSignature(CYContext &context) const {
    return MessageType(context, type_, parameters_, $S("@:"));
}

CYFunctionParameter *CYMessageParameter::Parameters(CYContext &context) const { $T(NULL)
    CYFunctionParameter *next(next_->Parameters(context));
    return name_ == NULL ? next : $ CYFunctionParameter($ CYDeclaration(name_), next);
}

CYSelector *CYMessageParameter::Selector(CYContext &context) const {
    return $ CYSelector(SelectorPart(context));
}

CYSelectorPart *CYMessageParameter::SelectorPart(CYContext &context) const { $T(NULL)
    CYSelectorPart *next(next_->SelectorPart(context));
    return tag_ == NULL ? next : $ CYSelectorPart(tag_, name_ != NULL, next);
}

CYExpression *CYMessageParameter::TypeSignature(CYContext &context) const {
    return MessageType(context, type_, next_);
}

CYExpression *CYBox::Replace(CYContext &context) {
    return $C1($M($V("Instance"), $S("box")), value_);
}

CYExpression *CYObjCBlock::Replace(CYContext &context) {
    return $N2($V("Functor"), $ CYFunctionExpression(NULL, $ CYFunctionParameter($ CYDeclaration($ CYIdentifier("$cyt")), parameters_->Parameters(context)), statements_), parameters_->TypeSignature(context, $ CYAdd(type_->Replace(context), $ CYString("@"))));
}

CYStatement *CYProtocol::Replace(CYContext &context) const { $T(NULL)
    return $ CYBlock($$->*
        next_->Replace(context)->*
        $E($C2($V("class_addProtocol"),
            $V("$cyc"), name_
        ))
    );
}

CYExpression *CYSelector::Replace(CYContext &context) {
    return $C1($V("sel_registerName"), name_->Replace(context));
}

CYString *CYSelectorPart::Replace(CYContext &context) {
    std::ostringstream str;
    CYForEach (part, this) {
        if (part->name_ != NULL)
            str << part->name_->Word();
        if (part->value_)
            str << ':';
    }
    return $S($pool.strdup(str.str().c_str()));
}

CYExpression *CYSendDirect::Replace(CYContext &context) {
    std::ostringstream name;
    CYArgument **argument(&arguments_);
    CYSelectorPart *selector(NULL), *current(NULL);

    while (*argument != NULL) {
        if ((*argument)->name_ != NULL) {
            CYSelectorPart *part($ CYSelectorPart((*argument)->name_, (*argument)->value_ != NULL));
            if (selector == NULL)
                selector = part;
            if (current != NULL)
                current->SetNext(part);
            current = part;
            (*argument)->name_ = NULL;
        }

        if ((*argument)->value_ == NULL)
            *argument = (*argument)->next_;
        else
            argument = &(*argument)->next_;
    }

    return $C2($V("objc_msgSend"), self_, ($ CYSelector(selector))->Replace(context), arguments_);
}

CYExpression *CYSendSuper::Replace(CYContext &context) {
    return $ CYSendDirect($V("$cyr"), arguments_);
}

CYFunctionParameter *CYTypedParameter::Parameters(CYContext &context) { $T(NULL)
    return $ CYFunctionParameter($ CYDeclaration(typed_->identifier_ ?: context.Unique()), next_->Parameters(context));
}

CYExpression *CYTypedParameter::TypeSignature(CYContext &context, CYExpression *prefix) { $T(prefix)
    return next_->TypeSignature(context, $ CYAdd(prefix, typed_->type_->Replace(context)));
}
