/*
 * В классе Octopus задать пользовательский конструктор копирования.
 * Копирующий конструктор, сгенерированный компилятором, копирует осьминогов некорректно: несколько осьминогов используют щупальца прототипа. Это приводит к неопределённому поведению.
 * Реализуйте следующий функционал:
 * - Корректное клонирование осьминогов, при котором копия будет иметь свой набор щупалец, а не пользоваться щупальцами оригинала.
 * - Щупальца копии осьминога должны прицепляться к тем же щупальцам, что и оригинал.
 * Разработайте метод AddTentacle для добавления новых щупалец. Добавление должно сохранять адрес размещения существующих щупалец в памяти.
 * id щупальца должен быть равен текущему количеству щупалец, увеличенному на 1.
 * Гарантируется, что количество щупалец, передаваемое в параметризованный конструктор класса Octopus, неотрицательное.
 * Проверять их количество в конструкторе необязательно. Максимальное количество щупалец, которое будет иметь осьминог, не превысит несколько десятков.
 */

#include "octopus.h"

#include <cassert>
#include <iostream>

using namespace std::literals;

int main() {
    // Проверка конструирования осьминогов
    {
        // По умолчанию осьминог имеет 8 щупалец
        Octopus default_octopus;
        assert(default_octopus.GetTentacleCount() == 8);

            std::cout << "1 По умолчанию осьминог имеет 8 щупалец"s << std::endl;

        // Осьминог может иметь отличное от 8 количество щупалец
        Octopus quadropus(4);
        assert(quadropus.GetTentacleCount() == 4);

            std::cout << "2 Осьминог может иметь отличное от 8 количество щупалец"s << std::endl;

        // И даже вообще не иметь щупалец
        Octopus coloboque(0);
        assert(coloboque.GetTentacleCount() == 0);

            std::cout << "3 И даже вообще не иметь щупалец"s << std::endl;
    }

    // Осьминогу можно добавлять щупальца
    {
        Octopus octopus(1);
        Tentacle *t0 = &octopus.GetTentacle(0);
        Tentacle *t1 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 2);

            std::cout << "4 Осьминогу можно добавлять щупальца"s << std::endl;

        Tentacle *t2 = &octopus.AddTentacle();
        assert(octopus.GetTentacleCount() == 3);

            std::cout << "5 Осьминогу можно добавлять щупальца"s << std::endl;

        // После добавления щупалец ранее созданные щупальца не меняют своих адресов
        assert(&octopus.GetTentacle(0) == t0);
        assert(&octopus.GetTentacle(1) == t1);
        assert(&octopus.GetTentacle(2) == t2);

            std::cout << "6 После добавления щупалец ранее созданные щупальца не меняют своих адресов 1"s << std::endl;

        for (int i = 0; i < octopus.GetTentacleCount(); ++i) {
            assert(octopus.GetTentacle(i).GetId() == i + 1);
        }

            std::cout << "7 После добавления щупалец ранее созданные щупальца не меняют своих адресов 2"s << std::endl;
    }

    // Осьминоги могут прицепляться к щупальцам друг друга
    {
        Octopus male(2);
        Octopus female(2);

        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);

        male.GetTentacle(0).LinkTo(female.GetTentacle(1));
        assert(male.GetTentacle(0).GetLinkedTentacle() == &female.GetTentacle(1));

        male.GetTentacle(0).Unlink();
        assert(male.GetTentacle(0).GetLinkedTentacle() == nullptr);
    }

    // Копия осьминога имеет свою собственную копию щупалец, которые
    // копируют состояние щупалец оригинального осьминога
    {
        // Перебираем осьминогов с разным количеством щупалец
        for (int num_tentacles = 0; num_tentacles < 10; ++num_tentacles) {
            Octopus male(num_tentacles);
            Octopus female(num_tentacles);
            // Пусть они хватают друг друга за щупальца
            for (int i = 0; i < num_tentacles; ++i) {
                male.GetTentacle(i).LinkTo(female.GetTentacle(num_tentacles - 1 - i));
            }

            Octopus male_copy(male);
            // Проверяем состояние щупалец копии
            assert(male_copy.GetTentacleCount() == male.GetTentacleCount());
            for (int i = 0; i < male_copy.GetTentacleCount(); ++i) {
                // Каждое щупальце копии размещается по адресу, отличному от адреса оригинального щупальца
                assert(&male_copy.GetTentacle(i) != &male.GetTentacle(i));
                // Каждое щупальце копии прицепляется к тому же щупальцу, что и оригинальное
                assert(male_copy.GetTentacle(i).GetLinkedTentacle() == male.GetTentacle(i).GetLinkedTentacle());
            }
        }
        // Если вы видите эту надпись, то разрушение осьминогов, скорее всего,
        // прошло без неопределённого поведения
        std::cout << "Everything is OK"s << std::endl;
    }
}