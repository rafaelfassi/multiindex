#include <iostream>
#include <string>
#include "../../include/multiindex.h"

struct People {
    People(int _id, std::string _name, std::string _email, int _age, double _height) :
    id(_id), name(_name), email(_email), age(_age), height(_height) {}

    int id;
    std::string name;
    std::string email;
    int age;
    double height;
};

void printVal(const People &p)
{
    std::cout << p.id << " - " << p.name << " - " << p.email << " - " << p.age << " - " << p.height << std::endl;
}

int main()
{
    MultiIndex<People> mi;
    mi.addIndex_Ordered_Unique(&People::id);
    mi.addIndex_Hashed_NonUnique(&People::name);
    mi.addIndex_Hashed_Unique(&People::email);
    mi.addIndex_Ordered_NonUnique(&People::name, &People::age, &People::height); // A composition cannot be hashed type

    // mi.reserve(7); // Best performance for insert

    mi.addData(People(0, "Rafael", "rafa1@email.com", 35, 1.70));
    mi.addData(People(1, "Fernanda", "fer1@email.com", 28, 1.62));
    mi.addData(People(2, "Rafael", "rafa2@email.com", 35, 1.64));
    mi.addData(People(3, "Paula", "paul1@email.com", 26, 1.58));
    mi.addData(People(4, "Paula", "paul2@email.com", 26, 1.80));
    mi.addData(People(5, "Rafael", "rafa3@email.com", 35, 1.70));
    mi.addData(People(6, "Fernanda", "fer2@email.com", 20, 1.50));


    std::cout << "Find by id = 3" << std::endl;
    auto idxId = mi.getIndex(&People::id);
    if(idxId)
    {
        People *p = idxId->findFirst(3);
        if(p) printVal(*p);
    }

    std::cout << std::endl;
    std::cout << "Find for 'Fernanda'" << std::endl;
    auto idxName = mi.getIndex(&People::name);
    if (idxName)
    {
        auto srtKey = "Fernanda";
        for (auto it = idxName->find(srtKey); !it->is_end() && it->key() == srtKey; it->next())
            printVal(it->value());
    }

    std::cout << std::endl;
    std::cout << "Find by email = 'paul2@email.com'" << std::endl;
    auto idxEmail = mi.getIndex(&People::email);
    if(idxEmail)
    {
        People *p = idxEmail->findFirst("paul2@email.com");
        if(p) printVal(*p);
    }

    std::cout << std::endl;
    std::cout << "Find for 'Rafael' with age = 35 and height = 1.70" << std::endl;
    auto idxNameAgeHeight = mi.getIndex(&People::name, &People::age, &People::height);
    if (idxNameAgeHeight)
    {
        auto srtKey = std::make_tuple("Rafael", 35, 1.70);
        for (auto it = idxNameAgeHeight->find(srtKey); !it->is_end() && it->key() == srtKey; it->next())
            printVal(it->value());
    }

  return 0;
}
