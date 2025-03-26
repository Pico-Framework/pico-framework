#include "json.hpp"

#include <iostream>

#include <ff_utils.h>
#include <ff_stdio.h>
#include "controller.hpp"
#include "scheduler.hpp"

#include "serializer.hpp"

#define JSON_NOEXCEPTION

Serializer::Serializer(const std::string& filename) {

    if (!mount(DEVICENAME)) 
        panic("Failed to mount disk \n");  
    else {
        printf("Disk mounted successfully\n");      
        read_file(filename); 
    }          
}

void Serializer::read_file(const std::string& filename){

    std::string filebuff;

    // Opening file in reading mode
    FF_FILE* ptr = ff_fopen(filename.c_str(), "r");
    if (ptr){
        ff_fseek(ptr, 0, SEEK_END);
        long fsize = ff_ftell(ptr);
        ff_fseek(ptr, 0, SEEK_SET);  /* same as rewind(f); */
        printf("sprinkler settings size: %d\n", fsize);
        filebuff.resize(fsize);
        ff_fread((void*)&filebuff[0], fsize, 1, ptr);
        ff_fclose(ptr);
        parsed_data = parse(filebuff);
    }
    else{
        printf("Failed to open file: %s\n", filename.c_str());
    }
}

void Serializer::write_file(const std::string& filename, json j){
    
    std::string serialized_data = j.dump(4);
    // Opening file in writingg mode
    FF_FILE* ptr = ff_fopen(filename.c_str(), "w");
    if (ptr){
        ff_fwrite((void*)serialized_data.c_str(), serialized_data.size(), 1, ptr);
        ff_fclose(ptr);
    }
    else{
        printf("Failed to open file: %s\n", filename.c_str());
    }
}

json Serializer::parse(std::string& filebuff){

    printf("%s", filebuff.c_str());
    if (filebuff.size() < 1)
        printf("filebuff is empty\n");
    else{
        printf("filebuff size is: %i\n", filebuff.size() ); 
        filebuff += "\0";
    }
    parsed_data = json::parse(filebuff);
    std::cout << parsed_data["sprinkler"]["settings"]["name"] << "\n\n";
    std::cout << parsed_data["sprinkler"]["settings"]["state"] << "\n\n";
    std::cout << parsed_data["sprinkler"]["zones"] << "\n\n";
    std::cout << parsed_data["sprinkler"]["programs"] << "\n\n";
    std::cout << parsed_data["sprinkler"]["zones"].at(2) << '\n';
    std::cout << parsed_data["sprinkler"]["programs"].at(0)["zone_run_times"] << "\n\n";
    std::cout << parsed_data["sprinkler"]["programs"].at(0)["zone_run_times"].at(0)["zone"] << "\n\n";
    return parsed_data;
}

bool Serializer::load_zones(){
    Controller* controller = Controller::get_instance();
    parsed_data["sprinkler"]["zones"].get_to(controller->zones);
    for (Zone zone: controller->zones)
        std::cout << zone.get_name() << std::endl;
    return true; // needs error checking - also need to turn off exceptions in json handling
}

bool Serializer::load_programs(){
    Scheduler* scheduler = Scheduler::get_instance();
    parsed_data["sprinkler"]["programs"].get_to(scheduler->programs);
    for (Program program: scheduler->programs){
        std::cout << program.get_name() << std::endl;
        std::cout << program.get_start_time_z() << std::endl;
    }
    return true; // needs error checking - also need to turn off exceptions in json handling
}

bool Serializer::load_settings(){
    Controller* controller = Controller::get_instance();
    parsed_data["sprinkler"]["settings"].get_to(controller->settings);
    std::cout << parsed_data["sprinkler"]["settings"]["name"] << "\n\n";
    std::cout << parsed_data["sprinkler"]["settings"]["status"] << "\n\n";
    std::cout << controller->settings.status << "\n\n";
    std::cout << controller->settings.name << "\n\n";
    return true; // needs error checking - also need to turn off exceptions in json handling
}

json Serializer::save_zones(){
    Controller* controller = Controller::get_instance();
    json j = controller->zones;
    std::cout << j << "\n\n";
    return j; // needs error checking - also need to turn off exceptions in json handling
}

json Serializer::save_programs(){
    Scheduler* scheduler = Scheduler::get_instance();
    json j = scheduler->programs;
    std::cout << j << "\n\n";
    return j; // needs error checking - also need to turn off exceptions in json handling
}

json Serializer::save_settings(){
    Controller* controller = Controller::get_instance();
    json j = controller->settings;
    std::cout << controller->settings.status << "\n\n";
    std::cout << controller->settings.name << "\n\n";
    std::cout << j << "\n\n";
    return j; // needs error checking - also need to turn off exceptions in json handling
}

json Serializer::save_all(){
    json settings = save_settings();
    json zones = save_zones();
    json programs = save_programs();
    
    json sprinkler = json::object();
    sprinkler["sprinkler"]["programs"] = programs;
    sprinkler["sprinkler"]["zones"] = zones;
    sprinkler["sprinkler"]["settings"] = settings;

    std::cout << sprinkler << "\n\n";
    return sprinkler; // needs error checking - also need to turn off exceptions in json handling
}