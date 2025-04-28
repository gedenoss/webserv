/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: anaouali <anaouali@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 15:18:07 by anaouali          #+#    #+#             */
/*   Updated: 2024/10/07 15:38:17 by anaouali         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Zombie.hpp"

int main(int argc, char **argv){

    Zombie *pol;
    int i = 0;
    int num;
    
    if (argc != 3)
        return (std::cout << "ERORR : please input a number of Zombies and Zombie name" << std::endl, 0);

    std::istringstream iss(argv[1]);
    iss >> num;
    
    pol = zombieHorde(num, argv[2]);
    while (i < num){
        pol[i].annouce();
        i++;
    }
    delete[] pol;
}
