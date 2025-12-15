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
        res["еда"] = 80;
        res["дерево"] = 40;
        res["золото"] = 30;
        res["камень"] = 40;
        res["население"] = 40;
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

struct Choice {
    std::string description;
    std::function<void(Resource&, int&)> effect; // effect может изменять ресурсы и мораль
};

class EmergencyEvent {
public:
    std::string description;
    std::vector<Choice> choices;
    int probability;
    
    EmergencyEvent(std::string desc, std::vector<Choice> ch, int prob)
        : description(desc), choices(ch), probability(prob) {}
};

class Kingdom{

private:
    Resource resources; // ресурсы королевства
    std::vector<Building> available_buildings; // доступные типы зданий
    std::vector<Building> built_buildings; // уже построенные здания
    int current_year = 1; // счётчик хода
    int current_morale = 50; // от 0 до 100
    std::vector<Event> events; // возможные события
    std::vector<EmergencyEvent> emergency_events; // экстренные события
    bool emergency_happened_last_turn = false; // флаг экстренного события

    void initEmergencyEvents() {
        // Пожар на складе
        emergency_events.push_back(EmergencyEvent(
            "🔥 ПОЖАР НА СКЛАДЕ! Огонь угрожает запасам!",
            {
                {"Тушить водой из реки (бесплатно, но потеря 30 еды)", 
                 [](Resource& res, int& morale) { res.add("еда", -30); morale += 5; }},
                {"Нанять пожарную бригаду (-40 золото, потеря 10 еды)",
                 [](Resource& res, int& morale) { 
                     if(res.subtract("золото", 40)) { res.add("еда", -10); morale += 10; }
                     else { res.add("еда", -30); morale -= 5; }
                 }},
                {"Игнорировать (потеря 50 еды и -15 мораль)",
                 [](Resource& res, int& morale) { res.add("еда", -50); morale -= 15; }}
            }, 10
        ));
        
        // Эпидемия
        emergency_events.push_back(EmergencyEvent(
            "☠️ ЭПИДЕМИЯ! Болезнь распространяется среди населения!",
            {
                {"Карантин (-20 золото, -10 население, +5 мораль)",
                 [](Resource& res, int& morale) { res.subtract("золото", 20); res.add("население", -10); morale += 5; }},
                {"Нанять лекарей (-60 золото, -5 население)",
                 [](Resource& res, int& morale) {
                     if(res.subtract("золото", 60)) { res.add("население", -5); morale += 10; }
                     else { res.add("население", -25); morale -= 10; }
                 }},
                {"Ничего не делать (-30 население, -20 мораль)",
                 [](Resource& res, int& morale) { res.add("население", -30); morale -= 20; }}
            }, 8
        ));
        
        // Набег бандитов
        emergency_events.push_back(EmergencyEvent(
            "⚔️ НАБЕГ БАНДИТОВ! Разбойники у ворот!",
            {
                {"Отбиться силами народа (-15 население, -10 мораль)",
                 [](Resource& res, int& morale) { res.add("население", -15); morale -= 10; }},
                {"Откупиться (-50 золото, -30 еда)",
                 [](Resource& res, int& morale) { res.subtract("золото", 50); res.add("еда", -30); morale -= 5; }},
                {"Нанять наёмников (-80 золото, успех!)",
                 [](Resource& res, int& morale) {
                     if(res.subtract("золото", 80)) { morale += 15; }
                     else { res.add("население", -20); res.subtract("золото", 40); morale -= 15; }
                 }}
            }, 9
        ));
        
        // Засуха
        emergency_events.push_back(EmergencyEvent(
            "🌵 ЗАСУХА! Посевы засыхают!",
            {
                {"Рыть колодцы (-30 дерево, -20 камень, спасти часть урожая)",
                 [](Resource& res, int& morale) { 
                     if(res.subtract("дерево", 30) && res.subtract("камень", 20)) {
                         res.add("еда", -20); morale += 5;
                     } else {
                         res.add("еда", -60); morale -= 10;
                     }
                 }},
                {"Купить еду у соседей (-70 золото, +10 еда)",
                 [](Resource& res, int& morale) {
                     if(res.subtract("золото", 70)) { res.add("еда", 10); }
                     else { res.add("еда", -60); morale -= 15; }
                 }},
                {"Пережить (-60 еда, -15 мораль)",
                 [](Resource& res, int& morale) { res.add("еда", -60); morale -= 15; }}
            }, 9
        ));
        
        // Обрушение шахты
        emergency_events.push_back(EmergencyEvent(
            "⛏️ ОБРУШЕНИЕ ШАХТЫ! Рабочие под завалами!",
            {
                {"Организовать спасательную операцию (-25 дерево, -10 население)",
                 [](Resource& res, int& morale) { 
                     res.subtract("дерево", 25); 
                     res.add("население", -10); 
                     morale += 10; 
                 }},
                {"Нанять профессионалов (-50 золото, -5 население, +15 мораль)",
                 [](Resource& res, int& morale) {
                     if(res.subtract("золото", 50)) { 
                         res.add("население", -5); morale += 15; 
                     } else { 
                         res.add("население", -20); morale -= 20; 
                     }
                 }},
                {"Оставить шахту (-25 население, -25 мораль, -20 камень)",
                 [](Resource& res, int& morale) { 
                     res.add("население", -25); 
                     morale -= 25;
                     res.add("камень", -20);
                 }}
            }, 7
        ));
        
        // Нашествие крыс
        emergency_events.push_back(EmergencyEvent(
            "🐀 НАШЕСТВИЕ КРЫС! Грызуны уничтожают запасы!",
            {
                {"Нанять крысоловов (-30 золото, потеря 20 еды)",
                 [](Resource& res, int& morale) { 
                     if(res.subtract("золото", 30)) { 
                         res.add("еда", -20); 
                     } else { 
                         res.add("еда", -50); morale -= 10;
                     }
                 }},
                {"Использовать кошек из деревни (бесплатно, -35 еда)",
                 [](Resource& res, int& morale) { res.add("еда", -35); }},
                {"Ничего не делать (-50 еда, -10 мораль)",
                 [](Resource& res, int& morale) { res.add("еда", -50); morale -= 10; }}
            }, 8
        ));
        
        // Налёт диких зверей
        emergency_events.push_back(EmergencyEvent(
            "🐺 НАЛЁТ ДИКИХ ЗВЕРЕЙ! Волки нападают на окраины!",
            {
                {"Организовать охоту (-10 население, +20 еда)",
                 [](Resource& res, int& morale) { 
                     res.add("население", -10); 
                     res.add("еда", 20);
                     morale += 5;
                 }},
                {"Построить укрепления (-40 дерево, -25 камень)",
                 [](Resource& res, int& morale) {
                     if(res.subtract("дерево", 40) && res.subtract("камень", 25)) {
                         morale += 10;
                     } else {
                         res.add("население", -15); morale -= 10;
                     }
                 }},
                {"Игнорировать (-15 население, -15 еда)",
                 [](Resource& res, int& morale) { 
                     res.add("население", -15); 
                     res.add("еда", -15);
                     morale -= 10;
                 }}
            }, 7
        ));
        
        // Восстание ремесленников
        emergency_events.push_back(EmergencyEvent(
            "✊ ВОССТАНИЕ РЕМЕСЛЕННИКОВ! Требуют повышения зарплаты!",
            {
                {"Удовлетворить требования (-60 золото, +10 мораль)",
                 [](Resource& res, int& morale) {
                     if(res.subtract("золото", 60)) {
                         morale += 10;
                     } else {
                         res.add("население", -20); morale -= 15;
                     }
                 }},
                {"Договориться (-30 золото, -10 дерево)",
                 [](Resource& res, int& morale) {
                     res.subtract("золото", 30);
                     res.subtract("дерево", 10);
                     morale += 3;
                 }},
                {"Подавить силой (-25 население, -20 мораль, -15 золото)",
                 [](Resource& res, int& morale) { 
                     res.add("население", -25);
                     morale -= 20;
                     res.subtract("золото", 15);
                 }}
            }, 6
        ));
        
        // Ураган
        emergency_events.push_back(EmergencyEvent(
            "🌪️ УРАГАН! Сильный ветер разрушает постройки!",
            {
                {"Эвакуировать народ (бесплатно, -30 дерево, -5 население)",
                 [](Resource& res, int& morale) { 
                     res.add("дерево", -30);
                     res.add("население", -5);
                     morale -= 5;
                 }},
                {"Укрепить здания (-50 дерево, -30 камень, минимальный урон)",
                 [](Resource& res, int& morale) {
                     if(res.subtract("дерево", 50) && res.subtract("камень", 30)) {
                         res.add("дерево", -10);
                         morale += 5;
                     } else {
                         res.add("дерево", -40);
                         res.add("население", -10);
                         morale -= 10;
                     }
                 }},
                {"Переждать (-50 дерево, -20 камень, -10 население)",
                 [](Resource& res, int& morale) { 
                     res.add("дерево", -50);
                     res.add("камень", -20);
                     res.add("население", -10);
                     morale -= 10;
                 }}
            }, 6
        ));
        
        // Прибытие беженцев
        emergency_events.push_back(EmergencyEvent(
            "👥 БЕЖЕНЦЫ! Группа людей просит убежища!",
            {
                {"Принять всех (+30 население, -40 еда, -20 золото)",
                 [](Resource& res, int& morale) { 
                     if(res.subtract("еда", 40) && res.subtract("золото", 20)) {
                         res.add("население", 30);
                         morale += 15;
                     } else {
                         res.add("население", 15);
                         morale -= 5;
                     }
                 }},
                {"Принять частично (+15 население, -20 еда)",
                 [](Resource& res, int& morale) {
                     res.add("население", 15);
                     res.add("еда", -20);
                     morale += 5;
                 }},
                {"Отказать (-10 мораль)",
                 [](Resource& res, int& morale) { morale -= 10; }}
            }, 8
        ));
    }

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
        available_buildings.push_back(Building("Ферма", {{"дерево", 20}, {"камень", 10}}, {{"еда", 20}}, {}));
        available_buildings.push_back(Building("Лесопилка", {{"дерево", 15}, {"золото", 5}}, {{"дерево", 15}}, {}));
        available_buildings.push_back(Building("Золотой рудник", {{"дерево", 25}, {"камень", 15}}, {{"золото", 12}}, {}));
        available_buildings.push_back(Building("Каменоломня", {{"дерево", 15}, {"золото", 10}}, {{"камень", 18}}, {}));
        available_buildings.push_back(Building("Рынок", {{"дерево", 30}, {"золото", 20}}, {{"золото", 8}}, {}));
        available_buildings.push_back(Building("Жилые дома", {{"дерево", 25}, {"камень", 20}}, {{"население", 10}}, {{"еда", 3}}));
    }

public:

    Kingdom(){
        std::srand(std::time(NULL)); // инициализация генератора случайных чисел
        initEvents();
        initEmergencyEvents();
        initBuildings();
    }

    void displayStatus(){
        std::cout << "\n========================================" << std::endl;
        std::cout << "Год: " << current_year << ", Мораль: " << current_morale << "/100" << std::endl;
        resources.print_resources();
        std::cout << "\nПостроенные здания (" << built_buildings.size() << "):" << std::endl;
        if (built_buildings.empty()) {
            std::cout << "  (пока нет построек)" << std::endl;
        } else {
            for(const auto &building : built_buildings){
                std::cout << "  - " << building.get_name() << std::endl;
            }
        }
        std::cout << "========================================\n" << std::endl;
    }

    // Производство и потребление ресурсов зданиями
    void produceResources(){
        // БАЗОВОЕ ПОТРЕБЛЕНИЕ: население ест еду каждый ход!
        int food_consumption = resources.res["население"] / 4; // каждые 4 человека едят 1 еду
        if (food_consumption < 8) food_consumption = 8; // минимум 8 еды в ход
        
        resources.add("еда", -food_consumption);
        std::cout << "Население потребило " << food_consumption << " еды." << std::endl;
        
        // Производство зданий
        for (auto& building : built_buildings){
            // сначала вычитаем потребляемые ресурсы
            for (auto& cons : building.consumption){
                resources.subtract(cons.first, cons.second);
            }

            // затем добавляем производимые ресурсы
            for (auto& prod : building.production){
                resources.add(prod.first, prod.second);
            }
        }

        // Проверка на нехватку еды
        if (resources.res["еда"] < 0) {
            std::cout << "⚠️ ГОЛОД! Не хватает еды!" << std::endl;
            current_morale -= 15;
            resources.res["население"] -= 10;
            resources.res["еда"] = 0; // еда не может быть отрицательной
        }
        else if (resources.res["еда"] < 30) {
            current_morale -= 5;
            resources.res["население"] -= 2;
        } 
        else if (resources.res["еда"] > 120) {
            current_morale += 3;
            resources.res["население"] += 5;
        }
        
        // Ограничение морали
        if (current_morale > 100) current_morale = 100;
        if (current_morale < 0) current_morale = 0;
    }

    void handleEvents(){
        // Обычные события
        for (const auto &event : events){
            int roll = std::rand() % 100;

            if (roll < event.probability){
                std::cout << "Событие: " << event.description << std::endl;
                event.effect(resources);
            }
        }
        
        // Экстренные события с выбором - не чаще чем через ход
        if (!emergency_happened_last_turn) {
            // Перемешиваем события для большей случайности
            int emergency_check = std::rand() % 100;
            
            // Базовый шанс 20%, увеличивается при плохих условиях
            int base_chance = 20;
            if (current_morale < 40) base_chance += 10;
            if (resources.res["еда"] < 30) base_chance += 10;
            if (resources.res["золото"] < 30) base_chance += 5;
            
            if (emergency_check < base_chance) {
                // Выбираем случайное экстренное событие
                int event_index = std::rand() % emergency_events.size();
                const auto &emergency = emergency_events[event_index];
                
                std::cout << "\n" << std::endl;
                std::cout << "╔════════════════════════════════════════════════════╗" << std::endl;
                std::cout << "║         ЭКСТРЕННАЯ СИТУАЦИЯ!                       ║" << std::endl;
                std::cout << "╚════════════════════════════════════════════════════╝" << std::endl;
                std::cout << emergency.description << std::endl;
                std::cout << "\nВарианты действий:" << std::endl;
                
                for (size_t i = 0; i < emergency.choices.size(); i++){
                    std::cout << (i + 1) << ". " << emergency.choices[i].description << std::endl;
                }
                
                std::cout << "Ваш выбор (1-" << emergency.choices.size() << "): ";
                int choice;
                std::cin >> choice;
                
                if (choice > 0 && choice <= (int)emergency.choices.size()){
                    emergency.choices[choice - 1].effect(resources, current_morale);
                    std::cout << "Решение принято!" << std::endl;
                } else {
                    std::cout << "Неверный выбор! Ситуация разрешилась случайно..." << std::endl;
                    emergency.choices[0].effect(resources, current_morale);
                }
                
                emergency_happened_last_turn = true;
                
                std::cout << "\nНажмите Enter для продолжения...";
                std::cin.ignore();
                std::cin.get();
            }
        } else {
            emergency_happened_last_turn = false; // сбрасываем флаг
        }

        // Бунт при низкой морали
        if (current_morale < 30 && std::rand() % 100 < 25){
            std::cout << "⚠️ Из-за низкой морали в королевстве произошёл бунт! -20 к золоту и -20 к населению." << std::endl;
            resources.add("золото", -20);
            resources.add("население", -20);
        }
    }

    void buildMenu(){
        std::cout << "\n--- Меню строительства ---" << std::endl;
        int index = 1;
        for (const auto &building : available_buildings){
            std::cout << index << ". " << building.get_name() << " (";
            bool first = true;
            for (const auto &cost : building.get_cost()){
                if (!first) std::cout << ", ";
                std::cout << cost.first << ": " << cost.second;
                first = false;
            }
            std::cout << ")" << std::endl;
            index++;
        }
        std::cout << "0. Отмена" << std::endl;
        std::cout << "Выберите здание: ";

        int choice;
        std::cin >> choice;
        if (choice > 0 && choice <= (int)available_buildings.size()){
            Building selectedBuilding = available_buildings[choice - 1];
            bool canBuild = true;
            
            // Проверяем, хватает ли ресурсов
            for (const auto &cost : selectedBuilding.get_cost()){
                if (resources.res[cost.first] < cost.second){
                    canBuild = false;
                    std::cout << "Недостаточно ресурсов! Не хватает " << cost.first << "." << std::endl;
                    break;
                }
            }
            
            if (canBuild) {
                // Вычитаем ресурсы
                for (const auto &cost : selectedBuilding.get_cost()){
                    resources.subtract(cost.first, cost.second);
                }
                built_buildings.push_back(selectedBuilding);
                std::cout << "✓ " << selectedBuilding.get_name() << " успешно построено!" << std::endl;
            }
        }
    }

    void DecisionMenu(){
        std::cout << "\n--- Экономические решения ---" << std::endl;
        std::cout << "1. Раздать еду крестьянам (-50 еда, +10 мораль)" << std::endl;
        std::cout << "2. Продать ресурсы на рынке (-30 дерево, +40 золото)" << std::endl;
        std::cout << "3. Нанять рабочих (+15 население, -50 золото)" << std::endl;
        std::cout << "4. Продать камень (-40 камень, +35 золото)" << std::endl;
        std::cout << "5. Купить еду у торговцев (+40 еда, -45 золото)" << std::endl;
        std::cout << "6. Устроить фестиваль (-30 еда, -20 золото, +15 мораль)" << std::endl;
        std::cout << "7. Ввести налоги (+25 золото, -10 мораль)" << std::endl;
        std::cout << "8. Закупить стройматериалы (+20 дерево, +15 камень, -55 золото)" << std::endl;
        std::cout << "9. Отправить торговую экспедицию (-30 золото, шанс большой прибыли)" << std::endl;
        std::cout << "10. Провести реформы (-40 золото, +5 мораль, улучшение управления)" << std::endl;
        std::cout << "0. Пропустить" << std::endl;
        std::cout << "Ваш выбор: ";

        int choice;
        std::cin >> choice;
        switch (choice) {
        case 1:
            if (resources.subtract("еда", 50)) {
                current_morale += 10;
                if (current_morale > 100) current_morale = 100;
                std::cout << "Крестьяне благодарны! Мораль повысилась." << std::endl;
            } else {
                std::cout << "Недостаточно еды." << std::endl;
            }
            break;
        case 2:
            if (resources.subtract("дерево", 30)) {
                resources.add("золото", 40);
                std::cout << "Сделка прошла успешно! Получено золото." << std::endl;
            } else {
                std::cout << "Недостаточно дерева." << std::endl;
            }
            break;
        case 3:
            if (resources.subtract("золото", 50)) {
                resources.add("население", 15);
                std::cout << "Новые рабочие прибыли в королевство!" << std::endl;
            } else {
                std::cout << "Недостаточно золота." << std::endl;
            }
            break;
        case 4:
            if (resources.subtract("камень", 40)) {
                resources.add("золото", 35);
                std::cout << "Камень продан. Получено золото." << std::endl;
            } else {
                std::cout << "Недостаточно камня." << std::endl;
            }
            break;
        case 5:
            if (resources.subtract("золото", 45)) {
                resources.add("еда", 40);
                std::cout << "Еда куплена у торговцев!" << std::endl;
            } else {
                std::cout << "Недостаточно золота." << std::endl;
            }
            break;
        case 6:
            if (resources.subtract("еда", 30) && resources.subtract("золото", 20)) {
                current_morale += 15;
                if (current_morale > 100) current_morale = 100;
                std::cout << "Фестиваль прошёл отлично! Народ доволен!" << std::endl;
            } else {
                std::cout << "Недостаточно ресурсов." << std::endl;
            }
            break;
        case 7:
            if (current_morale > 20) {
                resources.add("золото", 25);
                current_morale -= 10;
                std::cout << "Налоги собраны, но народ недоволен." << std::endl;
            } else {
                std::cout << "Мораль слишком низкая для сбора налогов!" << std::endl;
            }
            break;
        case 8:
            if (resources.subtract("золото", 55)) {
                resources.add("дерево", 20);
                resources.add("камень", 15);
                std::cout << "Стройматериалы закуплены!" << std::endl;
            } else {
                std::cout << "Недостаточно золота." << std::endl;
            }
            break;
        case 9:
            if (resources.subtract("золото", 30)) {
                int success = std::rand() % 100;
                if (success < 60) {
                    int profit = 40 + (std::rand() % 30); // 40-70 золота
                    resources.add("золото", profit);
                    std::cout << "Экспедиция успешна! Получено " << profit << " золота!" << std::endl;
                } else if (success < 85) {
                    std::cout << "Экспедиция вернулась с пустыми руками..." << std::endl;
                } else {
                    resources.add("население", -5);
                    std::cout << "Экспедиция потерпела неудачу. Потеряно 5 человек." << std::endl;
                }
            } else {
                std::cout << "Недостаточно золота." << std::endl;
            }
            break;
        case 10:
            if (resources.subtract("золото", 40)) {
                current_morale += 5;
                if (current_morale > 100) current_morale = 100;
                std::cout << "Реформы проведены! Эффективность управления улучшена." << std::endl;
            } else {
                std::cout << "Недостаточно золота." << std::endl;
            }
            break;
        default:
            break;
        }
    }

    bool isGameOver(){
        return current_morale <= 0 || resources.res["население"] <= 0;
    }
    
    bool isVictory(){
        return resources.res["население"] >= 500 && resources.res["золото"] >= 1000;
    }

    void nextTurn(){
        current_year++;
        produceResources();
        handleEvents();
    }
    
    void showCityInfo(){
        std::cout << "\n╔════════════════════════════════════════════════════════════╗" << std::endl;
        std::cout << "║           ИНФОРМАЦИЯ О ГОРОДЕ                              ║" << std::endl;
        std::cout << "╚════════════════════════════════════════════════════════════╝" << std::endl;
        
        std::cout << "\n--- Текущие ресурсы ---" << std::endl;
        resources.print_resources();
        
        std::cout << "\n--- Постройки (" << built_buildings.size() << ") ---" << std::endl;
        if (built_buildings.empty()) {
            std::cout << "  (нет построек)" << std::endl;
        } else {
            for(const auto &building : built_buildings){
                std::cout << "  • " << building.name << std::endl;
            }
        }
        
        std::cout << "\n--- Производство за ход ---" << std::endl;
        std::map<std::string, int> total_production;
        std::map<std::string, int> total_consumption;
        
        for (const auto& building : built_buildings){
            for (const auto& prod : building.production){
                total_production[prod.first] += prod.second;
            }
            for (const auto& cons : building.consumption){
                total_consumption[cons.first] += cons.second;
            }
        }
        
        if (total_production.empty()) {
            std::cout << "  (нет производства)" << std::endl;
        } else {
            for (const auto& prod : total_production){
                std::cout << "  + " << prod.second << " " << prod.first << std::endl;
            }
        }
        
        std::cout << "\n--- Потребление за ход ---" << std::endl;
        if (total_consumption.empty()) {
            std::cout << "  (нет потребления)" << std::endl;
        } else {
            for (const auto& cons : total_consumption){
                std::cout << "  - " << cons.second << " " << cons.first << std::endl;
            }
        }
        
        std::cout << "\n--- Общий баланс ---" << std::endl;
        std::map<std::string, int> balance;
        for (const auto& prod : total_production){
            balance[prod.first] += prod.second;
        }
        for (const auto& cons : total_consumption){
            balance[cons.first] -= cons.second;
        }
        
        if (balance.empty()) {
            std::cout << "  (нейтральный баланс)" << std::endl;
        } else {
            for (const auto& bal : balance){
                if (bal.second > 0) {
                    std::cout << "  + " << bal.second << " " << bal.first << " в ход" << std::endl;
                } else if (bal.second < 0) {
                    std::cout << "  " << bal.second << " " << bal.first << " в ход" << std::endl;
                }
            }
        }
        
        std::cout << "\n--- Состояние королевства ---" << std::endl;
        std::cout << "  Год: " << current_year << std::endl;
        std::cout << "  Мораль: " << current_morale << "/100" << std::endl;
        
        std::cout << "\n════════════════════════════════════════════════════════════" << std::endl;
    }
    
    void showMainMenu(){
        std::cout << "\n--- Главное меню ---" << std::endl;
        std::cout << "1. Построить здание" << std::endl;
        std::cout << "2. Принять экономическое решение" << std::endl;
        std::cout << "3. Информация о городе" << std::endl;
        std::cout << "4. Завершить год (следующий ход)" << std::endl;
        std::cout << "5. Выход из игры" << std::endl;
        std::cout << "Ваш выбор: ";
    }
    
    void playGame(){
        std::cout << "\n======================================" << std::endl;
        std::cout << "  ДОБРО ПОЖАЛОВАТЬ В KINGDOM BUILDER" << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << "\nВы — правитель небольшого феодального государства." << std::endl;
        std::cout << "Ваша задача: развить королевство и достичь процветания!" << std::endl;
        std::cout << "\nЦель победы: 500 населения и 1000 золота" << std::endl;
        std::cout << "Поражение: мораль = 0 или население = 0\n" << std::endl;
        
        bool gameRunning = true;
        while (gameRunning && !isGameOver() && !isVictory()) {
            displayStatus();
            showMainMenu();
            
            int choice;
            std::cin >> choice;
            
            switch(choice) {
                case 1:
                    buildMenu();
                    break;
                case 2:
                    DecisionMenu();
                    break;
                case 3:
                    showCityInfo();
                    break;
                case 4:
                    std::cout << "\n--- Конец года " << current_year << " ---" << std::endl;
                    nextTurn();
                    std::cout << "\nНаступил год " << current_year << "!" << std::endl;
                    break;
                case 5:
                    gameRunning = false;
                    std::cout << "Вы покинули королевство..." << std::endl;
                    break;
                default:
                    std::cout << "Неверный выбор. Попробуйте снова." << std::endl;
                    break;
            }
        }
        
        // Проверка результата игры
        if (isVictory()) {
            std::cout << "\n" << std::endl;
            std::cout << "╔════════════════════════════════════╗" << std::endl;
            std::cout << "║          ПОБЕДА!                   ║" << std::endl;
            std::cout << "╚════════════════════════════════════╝" << std::endl;
            std::cout << "Ваше королевство достигло процветания!" << std::endl;
            std::cout << "Год: " << current_year << ", Население: " << resources.res["население"] 
                        << ", Золото: " << resources.res["золото"] << std::endl;

            std::cout << "Введите ENTER для выхода...";
            std::cin.ignore();

        } else if (isGameOver()) {
            std::cout << "\n" << std::endl;
            std::cout << "╔════════════════════════════════════╗" << std::endl;
            std::cout << "║        GAME OVER                   ║" << std::endl;
            std::cout << "╚════════════════════════════════════╝" << std::endl;
            if (current_morale <= 0) {
                std::cout << "Народ восстал! Ваше правление закончилось." << std::endl;
            } else if (resources.res["население"] <= 0) {
                std::cout << "Население вымерло. Королевство опустело." << std::endl;
            }
            std::cout << "Введите ENTER для выхода...";
            std::cin.ignore();
        }
    }
};

int main(){
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
    Kingdom kingdom;
    kingdom.playGame();
    return 0;
}