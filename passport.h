#pragma once

#include "identity_document.h"
#include <iostream>
#include <string>
#include <ctime>

using namespace std::string_view_literals;

class Passport {
public:
    Passport()
        : expiration_date_(GetExpirationDate())
    {
        Passport::SetVTable(this);
        std::cout << "Passport::Ctor()"sv << std::endl;
    }

    Passport(const Passport& other)
        : identity_document_(other.identity_document_)
        , expiration_date_(other.expiration_date_)
    {
        Passport::SetVTable(this);
        std::cout << "Passport::CCtor()"sv << std::endl;
    }

    ~Passport() {
        std::cout << "Passport::Dtor()"sv << std::endl;
        IdentityDocument::SetVTable((IdentityDocument*)this);
    }

    static void PrintUniqueIDCount() {
        IdentityDocument::PrintUniqueIDCount();
    }

    virtual void PrintVisa(const std::string& country) const {
        std::cout << "Passport::PrintVisa("sv << country << ") : "sv << GetID() << std::endl;
    }

    using DeleteFunction = void (*)(Passport *);
    using PrintFunction = void (*)(const Passport *);

    struct Vtable {
        DeleteFunction delete_this;
        PrintFunction print_this;
    };

    static void SetVTable(Passport *obj) {
        *(Passport::Vtable **)obj = &Passport::VTABLE;
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
        return (const Passport::Vtable *)identity_document_.GetVtable();
    }

    Vtable *GetVtable() {
        return (Passport::Vtable *)identity_document_.GetVtable();
    }

    int GetID() const {
        return identity_document_.GetID();
    }

private:
    IdentityDocument identity_document_;

    static Passport::Vtable VTABLE;

    static void Delete(Passport* /*obj*/) {
        //delete obj;
    }

    static void PrintID(const Passport *obj) {
        std::cout << "Passport::PrintID() : "sv << obj->identity_document_.GetID();
        std::cout << " expiration date : "sv << obj->expiration_date_.tm_mday << "/"sv << obj->expiration_date_.tm_mon << "/"sv
                  << obj->expiration_date_.tm_year + 1900 << std::endl;
    }

    const struct tm expiration_date_;

    tm GetExpirationDate() {
	    time_t t = time(nullptr);
	    tm exp_date = *localtime(&t);
	    exp_date.tm_year += 10;
	    mktime(&exp_date);
	    return exp_date;
	}
};

Passport::Vtable Passport::VTABLE = { Passport::Delete, Passport::PrintID };