#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <functional>


class Resource{
    public:
    // Изначальные количества ресурсов
    std::map<std::string, int> res;
    Resource(){
        res["еда"] = 100;
        res["дерево"] = 50;
        res["золото"] = 20;
        res["камень"] = 50;
        res["население"] = 50;
    }

    // Добавляет указанное количество ресурса
    void add(const std::string &resource, int amount){
        res[resource] += amount;
    }

    // Возвращает true, если вычитание прошло успешно, иначе false
    bool subtract(const std::string &resource, int amount){
        if(res[resource] >= amount){
            res[resource] -= amount;
            return true;
        }
        return false;
    }

    void print_resources(){
        std::cout << "Ресурсы:" << std::endl;
        for(const auto &pair : res){
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
    }
};

class Building{
    public:
        std::string name;
        std::map<std::string, int> cost; // стоимость здания в ресурсах
        std::map<std::string, int> production; // ресурсы, которые здание производит за цикл
        std::map<std::string, int> consumption; // ресурсы, которые здание потребляет за цикл

        // очень простой конструктор
        Building(std::string name, std::map<std::string, int> cost,
                    std::map<std::string, int> production,
                    std::map<std::string, int> consumption)
                : name(name), cost(cost),
                production(production), consumption(consumption) {}

            std::string get_name() const{
            return this->name;
            }

            std::map<std::string, int> get_cost() const {
            return this->cost;
            }
};

class Event{
    public:
        std::string description;
        std::function<void(Resource&)> effect;
        int probability; // вероятность события в процентах (0-100)
        // Конструктор события
        Event(std::string description, std::function<void(Resource&)> effect, int probability)
            : description(description), effect(effect), probability(probability) {}

};

class Kingdom{

private:
    Resource resources; // ресурсы королевства
    std::vector<Building> buildings; // построенные здания
    int current_year = 1; // счётчик хода
    int current_morale = 50; // от 0 до 100
    std::vector<Event> events; // возможные события

    void initEvents(){
        events.push_back(Event("Хороший урожай увеличил запасы еды на 50 ед.", 
            [](Resource &res){ res.add("еда", 50); }, 20));
        events.push_back(Event("Чума в городе! -20 к населению.", 
            [](Resource &res){ res.add("население", -20); }, 10));
        events.push_back(Event("Бунт из-за нехватки еды! -30 к золоту.", 
            [](Resource &res){ res.add("золото", -30); }, 15));
    }

    void initBuildings(){
        // имя, стоимость, производство, потребление
        buildings.push_back(Building("Ферма", {{"дерево", 20}, {"камень", 10}}, {{"еда", 30}}, {}));
        buildings.push_back(Building("Лесопилка", {{"дерево", 15}, {"золото", 5}}, {{"дерево", 20}}, {}));
        buildings.push_back(Building("Золотой рудник", {{"дерево", 25}, {"камень", 15}}, {{"золото", 15}}, {}));
    }

public:

    Kingdom(){
        std::srand(std::time(NULL)); // инициализация генератора случайных чисел
        initEvents();
        initBuildings();
    }

    void displayStatus(){
        std::cout << "Год: " << current_year << ", Мораль: " << current_morale << "/100" << std::endl;
        resources.print_resources();
        std::cout << "Построенные здания:" << std::endl;
        for(const auto &building : buildings){
            if (building.name != "") std::cout << building.get_name() << std::endl;
        }
    }

    // Производство и потребление ресурсов зданиями
    void produceResources(){
        for (auto& building : buildings){
            // сначала вычитаем потребляемые ресурсы
            for (auto& cons : building.consumption){
                resources.subtract(cons.first, cons.second);
            }

            // затем добавляем производимые ресурсы
            for (auto& prod : building.production){
                resources.add(prod.first, prod.second);
            }
        }

        if (resources.res["еда"] < 20) {
            current_morale -= 10; // снижение морали из-за нехватки еды
            resources.res["население"] -= 5; // снижение населения из-за голода

        } 
        else if (resources.res["еда"] > 50) {
            current_morale += 5; // повышение морали из-за изобилия еды
            resources.res["население"] += 10; // увеличение населения
        }
        else {
            current_morale += 0; // мораль стабильна
        }
    }

    void handleEvents(){
        for (const auto &event : events){
            int roll = std::rand() % 100;

            if (roll < event.probability){
                std::cout << "Событие: " << event.description << std::endl;
                event.effect(resources);
            }
        }

        int roll = std::rand() % 100;
        if (current_morale < 30 && std::rand() % 100 < 30){
            std::cout << "Из-за низкой морали в королевстве произошёл бунт! -20 к золоту и -20 к населению." << std::endl;
            resources.add("золото", -20);
            resources.add("население", -20);
        }
    }

    void buildMenu(){
        std::cout << "Меню строительства:" << std::endl;
        int index = 1;
        for (const auto &building : buildings){
            std::cout << index << ". " << building.get_name() << " (Стоимость: ";
            for (const auto &cost : building.get_cost()){
                std::cout << cost.first << ": " << cost.second << " ";
            }
            std::cout << ")" << std::endl;
            index++;
        }

        std::cout << "0. Отмена" << std::endl;

        int choice;
        std::cin >> choice;
        if (choice > 0 && choice <= buildings.size()){
            Building selectedBuilding = buildings[choice - 1];
            bool canBuild = true;
            for (const auto &cost : selectedBuilding.get_cost()){
                if (resources.subtract(cost.first, cost.second) == false){
                    canBuild = false;
                    std::cout << "Недостаточно ресурсов для строительства." << std::endl;
                    break;
                }
                else{
                    buildings.push_back(selectedBuilding);
                    std::cout << selectedBuilding.get_name() << " построено!" << std::endl;
                    break;
                }
            }
        }
    }

    void DecisionMenu(){
        std::cout << "Меню действий:" << std::endl;

        int choice;
        std::cin >> choice;
        switch (choice) {
        default:
            break;
        }
    }

    bool isGameOver(){
        return current_morale <= 0 || resources.res["население"] <= 0;

    void nextTurn(){
        produce();
        handleEvents();
        year++;
    }
};

int main(){
    std::cout << "Hello, world!";
    return 0;
}