#ifndef UTILS_H
#define UTILS_H

#include<set>
#include<algorithm>

#define REPRODUCIBLE 1

#define FatalError(s) {                                                \
    std::stringstream _where, _message;                                \
    _where << __FILE__ << ':' << __LINE__;                             \
    _message << std::string(s) + "\n" << __FILE__ << ':' << __LINE__;\
    std::cerr << _message.str() << "\nAborting...\n";                  \
    exit(EXIT_FAILURE);                                                \
}

/*
Given sets a and b it computes a \ b and save it in a
*/
template<typename T>
void set_difference_inplace(std::set<T>& a, const std::set<T>& b)
{
    std::set<T> c;
    std::set_difference(a.begin(), a.end(),b.begin(), b.end(), std::inserter(c, c.begin()) );
    a = c;
}

#endif /*UTILS_H*/