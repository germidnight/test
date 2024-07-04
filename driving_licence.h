#pragma once
/*
 * Нужно реализовать наследование без использования наследования.
 * Добавьте самостоятельно все нужные виртуальные функции и указатели на них, чтобы сохранить полиморфизм.
 */
#include "identity_document.h"
#include <iostream>
#include <string>

using namespace std::string_view_literals;

class DrivingLicence {
public:
    DrivingLicence() {
        DrivingLicence::SetVTable(this);
        std::cout << "DrivingLicence::Ctor()"sv << std::endl;
    }

    DrivingLicence(const DrivingLicence& other)
        : identity_document_(other.identity_document_)
    {
        DrivingLicence::SetVTable(this);
        std::cout << "DrivingLicence::CCtor()"sv << std::endl;
    }

    ~DrivingLicence() {
        std::cout << "DrivingLicence::Dtor()"sv << std::endl;
        IdentityDocument::SetVTable((IdentityDocument*)this);
    }

    static void PrintUniqueIDCount() {
        IdentityDocument::PrintUniqueIDCount();
    }

    using DeleteFunction = void (*)(DrivingLicence *);
    using PrintFunction = void (*)(const DrivingLicence *);

    struct Vtable {
        DeleteFunction delete_this;
        PrintFunction print_this;
    };

    static void SetVTable(DrivingLicence *obj) {
        *(DrivingLicence::Vtable **)obj = &DrivingLicence::VTABLE;
    }

    void PrintID() const {
        GetVtable()->print_this(this);
    }

    void Delete() {
        GetVtable()->delete_this(this);
    }

    operator IdentityDocument() {
        return {identity_document_};
    }

    const Vtable *GetVtable() const {
        return (const DrivingLicence::Vtable*)identity_document_.GetVtable();
    }

    Vtable *GetVtable() {
        return (DrivingLicence::Vtable*)identity_document_.GetVtable();
    }

    int GetID() const {
        return identity_document_.GetID();
    }

private:
    IdentityDocument identity_document_;

    static DrivingLicence::Vtable VTABLE;

    static void Delete(DrivingLicence* obj) {
        delete obj;
    }

    static void PrintID(const DrivingLicence* obj) {
        std::cout << "DrivingLicence::PrintID() : "sv << obj->identity_document_.GetID() << std::endl;
    }
};

DrivingLicence::Vtable DrivingLicence::VTABLE = { DrivingLicence::Delete, DrivingLicence::PrintID };