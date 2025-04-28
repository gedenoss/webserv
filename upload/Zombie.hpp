#ifndef ZOMBIE_HPP
# define ZOMBIE_HPP
#include <iostream>

class Zombie{

private:
    
    std::string name;

public:
    
    void annouce();
    void setName(std::string &name);

Zombie();
~Zombie();
};

void    randomChump(std::string name);
Zombie* newZombie(std::string name);

#endif

