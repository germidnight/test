/*
 * Реализуйте класс Any, который:
 * - имеет шаблонный конструктор, принимающий владение значением любого типа,
 * - имеет метод void Print(std::ostream& out) const, выводящий хранимое значение в поток out операцией <<.
 * Для решения задачи вам понадобятся вспомогательные классы. Попробуйте догадаться, какие именно.
 * Если не получится — заглядывайте в подсказку.
 * Вам пригодится псевдоним типа std::remove_reference_t.
 * Его можно применить, чтобы удалить из шаблонного типа все указания на ссылку
 *
 * При использовании вспомогательных абстрактных классов не забудьте добавить в них виртуальный деструктор.
 * Ограничения:
 * - Шаблонный конструктор должен принимать значение наиболее универсальным способом — Forwarding reference.
 * - Объект, переданный в конструктор Any, должен совершить только одно копирование или перемещение —
 * в зависимости от того, передаётся в функцию временный объект или постоянный.
 * - При удалении объекта Any должен удалиться и хранимый объект.
 * - Класс Any — не шаблонный.
 */

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

using namespace std;

class AnyStorageBase {
public:
    virtual ~AnyStorageBase() = default;
    virtual void Print(std::ostream &) const = 0;
};

template <typename T>
class AnyStorage : public AnyStorageBase {
public:
    template <typename Type>
    AnyStorage(Type&& object) {
        value_ = std::forward<Type>(object);
    }

    void Print(std::ostream& out) const override {
        out << value_;
    }

private:
    T value_;
};

class Any {
public:
    template <typename S>
    Any(S&& object) {
        using Initial = std::remove_reference_t<S>;
        ptr_ = std::make_unique<AnyStorage<Initial>>(std::forward<S>(object));
    }

    void Print(std::ostream& out) const {
        ptr_.get()->Print(out);
    }

    ~Any() {
        ptr_.release();
    }

private:
    std::unique_ptr<AnyStorageBase> ptr_;
};

class Dumper {
public:
    Dumper() {
        std::cout << "construct"sv << std::endl;
    }
    ~Dumper() {
        std::cout << "destruct"sv << std::endl;
    }
    Dumper(const Dumper &) {
        std::cout << "copy"sv << std::endl;
    }
    Dumper(Dumper &&) {
        std::cout << "move"sv << std::endl;
    }
    Dumper &operator=(const Dumper &) {
        std::cout << "= copy"sv << std::endl;
        return *this;
    }
    Dumper &operator=(Dumper &&) {
        std::cout << "= move"sv << std::endl;
        return *this;
    }
};

ostream &operator<<(ostream &out, const Any &arg) {
    arg.Print(out);
    return out;
}

ostream &operator<<(ostream &out, const Dumper &) {
    return out;
}

int main() {
    Any any_int(42);
    Any any_string("abc"s);

    // операция вывода Any в поток определена чуть выше в примере
    cout << any_int << endl
         << any_string << endl;

    Any any_dumper_temp{Dumper{}};

    Dumper auto_dumper;
    Any any_dumper_auto(auto_dumper);
}