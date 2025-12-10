#include <iostream>
#include <string>
#include <vector>

class event{
    public:

        int get_event_id(){
            return this->event_id;
        }

        void set_event_id(int new_event_id){
            this->event_id = new_event_id;
        }

        std::string get_event_group(){
            return this->event_group;
        }

        void set_event_group(std::string new_event_group){
            this->event_group = new_event_group;
        }


    private:
        int event_id;
        std::string event_group;
        
};


void main(){
    std::cout << "Hello, world!";
}