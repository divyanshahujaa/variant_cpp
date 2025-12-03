#include<stdio.h>
#include<stdlib.h>
#include <string>
#include <iostream>

#include "bootlegVariant.h"

int main(){
    variant<int, float, std::string> x;

    x.set<std::string>("rfg");
    std::cout<<x.get<std::string>();
    return 0;
}