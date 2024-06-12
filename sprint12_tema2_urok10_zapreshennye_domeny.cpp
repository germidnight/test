/*
 * Директор одной из школ города N, проанализировав историю браузера на компьютере в классе информатики,
 * был неприятно удивлён тем, какие сайты посещают ученики.
 * Он поручил немедленно установить на всех компьютерах школы фильтр доменов, чтобы заблокировать нежелательные сайты.
 * Реализовать фильтр доменов поручено вам. Он должен отфильтровать не только сам домен, но и поддомены.
 * Например, если запрещён домен gdz.ru, то должны быть отфильтрованы домены math.gdz.ru, history.gdz.ru, biology.gdz.ru, но не freegdz.ru.
 * Каждый домен состоит из одного или нескольких непустых слов, записанных латинскими буквами. Слова разделены точками.
 * Чтобы убедиться, что фильтр качественный, сделайте юнит-тесты.
 *
 * Разработайте класс Domain, задающий домен. Этот класс должен:
 * - позволять конструирование из объекта string;
 * - определять operator==;
 * - иметь метод IsSubdomain, принимающий другой домен и возвращающий bool, если this его поддомен.
 *
 * Разработайте функцию ReadDomains, которая читает из входного файла заданное количество доменов — по одному на строке.
 * Её параметры:
 * - поток istream, из которого нужно прочитать домены;
 * - количество доменов.
 *
 * Разработайте класс DomainChecker со следующими методами:
 * - Конструктор. Принимает запрещённые домены. Конструктор должен принимать список доменов через пару итераторов.
 * - Метод bool IsForbidden, принимающий домен и возвращающий true в случае, если он запрещён.
 *
 * Ограничения:
 * 1) Код должен быть качественным и удовлетворять критериям взаимной проверки, а сигнатуры функций и методов
 * должны соответствовать всем практикам и рекомендациям этого спринта.
 * 2) Сложность проверки одного домена — O(L⋅logN), где L — максимальная длина домена в символах, а N — количество запрещённых доменов.
 * Перебирать все запрещённые домены при проверке — долго.
 *
 * Формат входных данных:
 * - В первой строке написано количество запрещённых доменов.
 * - Далее перечислены запрещённые домены, каждый на отдельной строке.
 * - На следующей строке указано количество проверяемых доменов.
 * - Далее перечислены проверяемые домены, каждый на отдельной строке.
 *
 * Формат выходных данных:
 * Вывод уже содержится в заготовке кода.
 * - Выведите вердикт для каждого проверяемого домена на отдельной строке - выводите Bad, если домен запрещён, и Good, если разрешён.
 */

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using namespace std::literals;

class Domain {
public:
    Domain(std::string&& domain) : domain_(std::move(domain)) {
        std::reverse(domain_.begin(), domain_.end());
        domain_.push_back('.');
    }

    bool operator==(const Domain& other) const {
        return (domain_.compare(other.domain_) == 0);
    }

    bool operator<(const Domain& other) const {
        return (domain_ < other.domain_);
    }

    bool IsSubdomain(const Domain& subdomain) const {
        const size_t length = domain_.size();

        if (length > subdomain.domain_.size()) {
            return false;
        }
        if (subdomain.domain_.substr(0, length).compare(domain_) == 0) {
            return true;
        }
        return false;
    }
private:
    std::string domain_;
};

class DomainChecker {
public:
    // конструктор должен принимать список запрещённых доменов через пару итераторов
    template <typename IterType>
    DomainChecker(const IterType& begin, const IterType& end) : forbidden_domains_(begin, end) {
        std::sort(forbidden_domains_.begin(), forbidden_domains_.end(), [](const Domain& first, const Domain& second) {
                            return first < second;
                        });
        auto last = std::unique(forbidden_domains_.begin(), forbidden_domains_.end(), [](const Domain &first, const Domain &second) {
                            return first.IsSubdomain(second);
                        });
        forbidden_domains_.erase(last, forbidden_domains_.end());
    }

    // разработайте метод IsForbidden, возвращающий true, если домен запрещён
    bool IsForbidden(const Domain& domain_candidate) const {
        if (!forbidden_domains_.empty()) {
            auto it = std::find_if(forbidden_domains_.begin(), forbidden_domains_.end(), [domain_candidate](const Domain& domain) {
                            return domain.IsSubdomain(domain_candidate);
                        });
            if (it != forbidden_domains_.end()) {
                return true;
            }

        }
        return false;
    }
private:
    std::vector<Domain> forbidden_domains_;
};

// разработайте функцию ReadDomains, читающую заданное количество доменов из стандартного входа
std::vector<Domain> ReadDomains(std::istream& input, size_t count) {
    std::vector<Domain> domains;

    for (size_t i = 0; i != count; ++i) {
        std::string line;
        getline(input, line);
        std::string domain_str;
        std::istringstream(line) >> domain_str;
        domains.emplace_back(Domain(std::move(domain_str)));
    }
    return domains;
}

template <typename Number>
Number ReadNumberOnLine(std::istream &input) {
    std::string line;
    getline(input, line);

    Number num;
    std::istringstream(line) >> num;

    return num;
}

void TestIsSubdomain() {
    Domain domain1{"gdz.ru"s};
    assert(domain1.IsSubdomain(Domain{"gdz.ru"}));
    assert(domain1.IsSubdomain(Domain{"m.gdz.ru"}));
    assert(!domain1.IsSubdomain(Domain{"m.gdz.su"}));
    assert(!domain1.IsSubdomain(Domain{"fgdz.ru"}));
    assert(!domain1.IsSubdomain(Domain{"gdz.ru.com"}));

    Domain domain2{"com"s};
    assert(!domain2.IsSubdomain(Domain{"gdz.ru"}));
    assert(domain2.IsSubdomain(Domain{"com"}));
    assert(domain2.IsSubdomain(Domain{"ru.com"}));
    assert(domain2.IsSubdomain(Domain{"com.com"}));
    assert(!domain2.IsSubdomain(Domain{"com.ru"}));

    std::cout << "TestIsSubdomain() ok"s << std::endl;
}

int main() {
    //TestIsSubdomain();
    const std::vector<Domain> forbidden_domains = ReadDomains(std::cin, ReadNumberOnLine<size_t>(std::cin));
    DomainChecker checker(forbidden_domains.begin(), forbidden_domains.end());

    const std::vector<Domain> test_domains = ReadDomains(std::cin, ReadNumberOnLine<size_t>(std::cin));
    for (const Domain &domain : test_domains) {
        std::cout << (checker.IsForbidden(domain) ? "Bad"sv : "Good"sv) << std::endl;
    }
}