#pragma once

#include "driving_licence.h"
#include <iostream>
#include <string>

using namespace std::string_view_literals;

class InternationalDrivingLicence {
public:
    InternationalDrivingLicence() {
        InternationalDrivingLicence::SetVTable(this);
        std::cout << "InternationalDrivingLicence::Ctor()"sv << std::endl;
    }

    InternationalDrivingLicence(const InternationalDrivingLicence& other)
        : driving_license_(other.driving_license_)
    {
        InternationalDrivingLicence::SetVTable(this);
        std::cout << "InternationalDrivingLicence::CCtor()"sv << std::endl;
    }

    ~InternationalDrivingLicence() {
        std::cout << "InternationalDrivingLicence::Dtor()"sv << std::endl;
        DrivingLicence::SetVTable((DrivingLicence*)this);
    }

    static void PrintUniqueIDCount() {
        DrivingLicence::PrintUniqueIDCount();
    }

    using DeleteFunction = void(*)(InternationalDrivingLicence*);
    using PrintFunction = void (*)(const InternationalDrivingLicence *);

    struct Vtable {
        DeleteFunction delete_this;
        PrintFunction print_this;
    };

    static void SetVTable(InternationalDrivingLicence* obj) {
        *(InternationalDrivingLicence::Vtable**) obj = &InternationalDrivingLicence::VTABLE;
    }

    void Delete() {
        GetVtable()->delete_this(this);
    }

    void PrintID() const {
        GetVtable()->print_this(this);
    }

    operator IdentityDocument() {
        return {driving_license_.operator IdentityDocument()};
    }

    operator DrivingLicence() {
        return {driving_license_};
    }

    const Vtable* GetVtable() const {
        return (const InternationalDrivingLicence::Vtable*)driving_license_.GetVtable();
    }

    Vtable* GetVtable() {
        return (InternationalDrivingLicence::Vtable*)driving_license_.GetVtable();
    }

private:
    DrivingLicence driving_license_;

    static InternationalDrivingLicence::Vtable VTABLE;

    static void Delete(InternationalDrivingLicence* obj) {
        delete obj;
    }

    static void PrintID(const InternationalDrivingLicence* obj) {
        std::cout << "InternationalDrivingLicence::PrintID() : "sv << obj->driving_license_.GetID() << std::endl;
    }
};

InternationalDrivingLicence::Vtable InternationalDrivingLicence::VTABLE = { InternationalDrivingLicence::Delete, InternationalDrivingLicence::PrintID };