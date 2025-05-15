#include "../include/fraction.h"

#include <cmath>
#include <numeric>
#include <regex>
#include <sstream>

#include <algorithm>
#include <iomanip>
#include <string>

big_int gcd(big_int a, big_int b) {
	if (a < 0) a = big_int(0) - a;
	if (b < 0) b = big_int(0) - b;

	while (b != 0) {
		big_int tmp = b;
		b = a % b;
		a = tmp;
	}

	return a;
}

void fraction::optimise() {
    if (_denominator == big_int(0)) {
        throw std::invalid_argument("Denominator is zero");;
    }
    
    if (_denominator < big_int(0)) {
        _numerator = big_int(0) - _numerator;
        _denominator = big_int(0) - _denominator;
    }
    
    big_int common_divisor = gcd(_numerator, _denominator);
    
    if (common_divisor != big_int(0) && common_divisor != big_int(1)) {
        _numerator = _numerator / common_divisor;
        _denominator = _denominator / common_divisor;
    }
}

template<std::convertible_to<big_int> f, std::convertible_to<big_int> s>
fraction::fraction(f &&numerator, s &&denominator) 
    : _numerator(forward<f>(numerator)), _denominator(forward<s>(denominator)) {
    optimise();
}

fraction::fraction(pp_allocator<big_int::value_type> alloc) 
: _numerator(0, alloc), _denominator(1, alloc) {}

fraction &fraction::operator+=(fraction const &other) & {
    big_int new_num = _numerator * other._denominator + other._numerator * _denominator;
    big_int new_den = _denominator * other._denominator;
    _numerator = new_num;
    _denominator = new_den;
    optimise();
    return *this;
}

fraction fraction::operator+(fraction const &other) const {
	fraction result = *this;
	result += other;
	return result;
}

fraction &fraction::operator-=(fraction const &other) & {
    big_int new_num = _numerator * other._denominator - other._numerator * _denominator;
    big_int new_den = _denominator * other._denominator;
    _numerator = new_num;
    _denominator = new_den;
    optimise();
    return *this;
}

fraction fraction::operator-(fraction const &other) const {
	fraction result = *this;
	result -= other;
	return result;
}

fraction &fraction::operator*=(fraction const &other) & {
	_numerator *= other._numerator;
	_denominator *= other._denominator;
	optimise();
	return *this;
}

fraction fraction::operator*(fraction const &other) const {
	fraction result = *this;
	result *= other;
	return result;
}

fraction &fraction::operator/=(fraction const &other) & {
	if (other._numerator == 0) {
		throw std::invalid_argument("//0");
	}

	_numerator *= other._denominator;
	_denominator *= other._numerator;
	optimise();
	return *this;
}

fraction fraction::operator/(fraction const &other) const {
	fraction result = *this;
	result /= other;
	return result;
}

fraction fraction::operator-() const {
	fraction result = *this;
	result._numerator = big_int(0) - result._numerator;
	result.optimise();
	return result;
}

bool fraction::operator==(fraction const &other) const noexcept {
	return _numerator == other._numerator && _denominator == other._denominator;
}

std::partial_ordering fraction::operator<=>(const fraction& other) const noexcept {
    big_int left = _numerator * other._denominator;
    big_int right = other._numerator * _denominator;
    if (left == right) return std::partial_ordering::equivalent;
    if (left < right) return std::partial_ordering::less;
    return std::partial_ordering::greater;
}

std::ostream &operator<<(std::ostream &stream, fraction const &obj) {
    stream << obj._numerator << "/" << obj._denominator;
    return stream;
}

std::istream &operator>>(std::istream &stream, fraction &obj) {
    std::string input;
    stream >> input;
    
    size_t slash_pos = input.find('/');
    if (slash_pos != std::string::npos) {
        std::string num_str = input.substr(0, slash_pos);
        std::string den_str = input.substr(slash_pos + 1);
        
        obj._numerator = big_int(num_str);
        obj._denominator = big_int(den_str);
    } else {
        size_t dot_pos = input.find('.');
        if (dot_pos != std::string::npos) {
            std::string int_part = input.substr(0, dot_pos);
            std::string frac_part = input.substr(dot_pos + 1);
            
            big_int numerator = big_int(int_part + frac_part);
			big_int denominator("1");
			for (size_t i = 0; i < frac_part.length(); ++i) {
				denominator *= big_int("10");
			}

            obj._numerator = numerator;
            obj._denominator = denominator;
        } else {
            obj._numerator = big_int(input);
            obj._denominator = big_int(1);
        }
    }
    
    obj.optimise();
    return stream;
}

std::string fraction::to_string() const {
	std::stringstream string;
	string << _numerator << "/" << _denominator;
	return string.str();
}

fraction fraction::sin(fraction const &epsilon) const {
	fraction result(0, 1);
	fraction term = *this;
	int n = 1;
	while (term >= epsilon || -term <= epsilon) {
		result += term;
		term = -term * (*this) * (*this);
		term /= fraction((n+1)*(n+2), 1);
		n += 2;
	}

	return result;
}

fraction fraction::cos(fraction const &epsilon) const {
	fraction result(1, 1);
	fraction term(1, 1);
	int n = 1;
	while (true) {
		term = -term * (*this) * (*this);
		term /= fraction((2 * n - 1) * ( 2 * n), 1);
		if (term <= epsilon || -term >= epsilon) {
			break;
		}
		result += term;
		n++;
	}

	return result;
}

fraction fraction::tg(fraction const &epsilon) const {
	auto cos = this->cos(epsilon * epsilon);
	if (cos._numerator == 0) {
		throw std::domain_error("tg unreal");
	}

	return this->sin(epsilon * epsilon) / cos;
}

fraction fraction::ctg(fraction const &epsilon) const {
	auto sine = this->sin(epsilon * epsilon);
	if (sine._numerator == 0) {
		throw std::domain_error("ctg unreal");
	}

	return this->cos(epsilon * epsilon) / sine;
}

fraction fraction::sec(fraction const &epsilon) const {
	auto cos = this->cos(epsilon);
	if (cos._numerator == 0) {
		throw std::domain_error("sec unreal");
	}

	return fraction(1, 1) / cos;
}

fraction fraction::cosec(fraction const &epsilon) const {
	auto sin = this->sin(epsilon);
	if (sin._numerator == 0) {
		throw std::domain_error("cosec unreal");
	}

	return fraction(1, 1) / sin;
}

fraction fraction::arcsin(fraction const &epsilon) const {
    if (*this < fraction(-1, 1) || *this > fraction(1, 1)) {
		throw std::domain_error("arcsin unreal");
	}
    
    fraction result(0_bi, 1_bi);
    fraction term = *this;
    int n = 1;
    
    while (term > epsilon || term < -epsilon) {
        result += term;
        n += 2;
        term = term * fraction((n-2)*(n-2), 1_bi) * (*this * *this) / fraction(n*(n-1), 1_bi);
    }
    
    return result;
}

fraction fraction::arccos(fraction const &epsilon) const {
    fraction pi_approx(355_bi, 113_bi);
    return pi_approx/fraction(2_bi, 1) - arcsin(epsilon);
}
////////////////////////////////
fraction fraction::arctg(fraction const &epsilon) const {
	if (_numerator < 0) return -(-*this).arctg(epsilon);

	if (*this > fraction(1, 1)) {
		return fraction(1, 2) - (fraction(1, 1) / *this).arctg(epsilon);
	}

	fraction result(0, 1);
	fraction term = *this;
	int n = 1;
	while (term > epsilon) {
		result += fraction((n % 2 == 0 ? -1 : 1), n) * term;
		n += 2;
		term *= *this * *this;
	}

	return result;
}

fraction fraction::arctg(fraction const &epsilon) const {
    fraction result(0_bi, 1_bi);
    fraction term = *this;
    int n = 1;
    
    while (term > epsilon || term < -epsilon) {
        result += term;
        n += 2;
        term = -term * (*this * *this) * fraction(n-2, n);
    }
    
    return result;
}

fraction fraction::arcctg(fraction const &epsilon) const {
	if (_numerator == 0) {
		throw std::domain_error("arcctg unreal");
	}

	return (fraction(1, 1) / *this).arctg(epsilon);
}

fraction fraction::arcsec(fraction const &epsilon) const {
	if (_numerator == 0) {
		throw std::domain_error("arcsec unreal");
	}

	return (fraction(1, 1) / *this).arccos(epsilon);
}

fraction fraction::arccosec(fraction const &epsilon) const {
	if (_numerator == 0) {
		throw std::domain_error("arccosec unreal");
	}

	return (fraction(1, 1) / *this).arcsin(epsilon);
}

fraction fraction::pow(size_t degree) const {
	if (degree < 0) {
		throw std::invalid_argument("Degree must be positive");
	}
	if (degree == 0) {
		return {1, 1};
	}

    fraction result(1_bi, 1_bi);
    for (size_t i = 0; i < degree; ++i) {
        result *= *this;
    }
    return result;
}

fraction fraction::root(size_t degree, fraction const &epsilon) const {
    if (degree == 0) {
        throw std::invalid_argument("Degree cannot be 0");
    }
	if (degree == 1) {
		return *this;
	}
    if (_numerator < 0_bi && degree % 2 == 0) {
        throw std::invalid_argument("Even root of negative number is undefined");
    }
    
    fraction x = *this; 
    fraction delta;
	fraction degree_frac(degree, 1);

    do {
        fraction next = ((degree_frac - fraction(1, 1)) * x + (*this / x.pow(degree - 1))) / degree_frac;
        delta = (next > x) ? (next - x) : (x - next);
        x = next;
    } while (delta > epsilon);

    return x;
}

fraction fraction::log2(fraction const &epsilon) const {
	if (_numerator <= 0 || _denominator <= 0) { 
		throw std::domain_error("log2 unreal");
	}
	return this->ln(epsilon) / fraction(2, 1).ln(epsilon);
}

fraction fraction::ln(fraction const &epsilon) const {   
    fraction result(0_bi, 1_bi);
    fraction x = *this;
    fraction e(2718281828459045_bi, 1000000000000000_bi); 
    
    while (x >= e) {
        x /= e;
        result += fraction(1_bi, 1_bi);
    }
    
    fraction term = (x - fraction(1_bi, 1_bi)) / (x + fraction(1_bi, 1_bi));
    fraction term_sq = term * term;
    fraction current_term = term;
    int n = 1;
    
    while (current_term > epsilon || current_term < -epsilon) {
        result += current_term * fraction(2_bi, n);
        n += 2;
        current_term *= term_sq * fraction(n-2, n);
    }
    
    return result;
}


fraction fraction::lg(fraction const &epsilon) const {
	if (_numerator <= 0 || _denominator <= 0) {
		throw std::domain_error("lg unreal");
	}
	return this->ln(epsilon) / fraction(10, 1).ln(epsilon);
}