#include <iostream>

/* This is my first written code
on c++, this calculates your current CGPA by 
taking in your last semester's CGPA and current 
SGPA. 
*/
int main() {
    double A,sg,n;
    std::cout << "Enter your previous CGPA: ";
    std::cin >> A;
    std::cout << "Enter current SGPA: ";
    std::cin >> sg;
    std::cout << "Enter semesters over: ";
    std::cin >> n;
    double cg = A*(n-1)/n + sg/n;
    std::cout << "CPGA: " << cg ;
    return 0;

}