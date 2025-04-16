//
// Created by Des Caldnd on 5/27/2024.
//

#include "../include/big_int.h"
#include <ranges>
#include <exception>
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>

bool is_zero(const auto& digits) {
    return digits.empty() || (digits.size() == 1 && digits.front() == 0);
}

void optimise(auto& digits) {
    if (digits.size() <= 1) return;
    if (digits.back() != 0) return;
    
    digits.pop_back();
    optimise(digits);
}

std::strong_ordering big_int::operator<=>(const big_int &other) const noexcept
{
    if (!_sign && other._sign) return std::strong_ordering::less;
    if (_sign && !other._sign) return std::strong_ordering::greater;

    bool is_it_pos = _sign;

    if (_digits.size() != other._digits.size()) 
    {
        if (is_it_pos) 
        {
            return _digits.size() <=> other._digits.size();
        } else 
        { 
            return other._digits.size() <=> _digits.size();
        }
    }

    for (int i = _digits.size() - 1; i >= 0; i--) 
    {
        if (_digits[i] != other._digits[i]) {
            if (is_it_pos) 
            {
                return _digits[i] <=> other._digits[i];
            } else 
            { 
                return other._digits[i] <=> _digits[i];            
            } 
        }
    }

    return std::strong_ordering::equal;
}

big_int::operator bool() const noexcept
{
	return !is_zero(_digits);
}

big_int &big_int::operator++() &
{
    return (*this).operator+=(1);
}


big_int big_int::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

big_int &big_int::operator--() &
{
    return (*this).operator-=(1);
}


big_int big_int::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

big_int &big_int::operator+=(const big_int &other) & {
	return plus_assign(other, 0);
}

big_int &big_int::operator-=(const big_int &other) & {
	return minus_assign(other, 0);
}

big_int big_int::operator+(const big_int &other) const {
	big_int tmp = *this;
	return tmp += other;
}

big_int big_int::operator-(const big_int &other) const {
	big_int tmp = *this;
	return tmp -= other;
}

big_int big_int::operator*(const big_int &other) const {
	big_int tmp = *this;
	return tmp *= other;
}

big_int big_int::operator/(const big_int &other) const {
	big_int tmp = *this;
	return tmp /= other;
}

big_int big_int::operator%(const big_int &other) const {
	big_int tmp = *this;
	return tmp %= other;
}

big_int big_int::operator&(const big_int &other) const {
	big_int tmp = *this;
	return tmp &= other;
}

big_int big_int::operator|(const big_int &other) const {
	big_int tmp = *this;
	return tmp |= other;
}

big_int big_int::operator^(const big_int &other) const {
	big_int tmp = *this;
	return tmp ^= other;
}

big_int big_int::operator<<(size_t shift) const {
	big_int tmp = *this;
	return tmp <<= shift;
}

big_int big_int::operator>>(size_t shift) const {
	big_int tmp = *this;
	return tmp >>= shift;
}

big_int &big_int::operator%=(const big_int &other) & {
    return operator-=((*this / other) * other);
}

big_int big_int::operator~() const {
	big_int result(*this);
	result._sign = !result._sign;
	
	for (auto &digit: result._digits) {
		digit = ~digit;
	}

	optimise(result._digits);
	return result;
}

big_int &big_int::operator&=(const big_int &other) & {

	for(size_t i = 0; i < _digits.size(); ++i) {
		_digits[i] &= i < other._digits.size() ? other._digits[i] : 0;
	}

	optimise(_digits);
	return *this;
}

big_int &big_int::operator|=(const big_int &other) & {
	size_t max_size = std::max(_digits.size(), other._digits.size());
	_digits.resize(max_size, 0);

	for (size_t i = 0; i < max_size; ++i) {
		_digits[i] |= (i < other._digits.size()) ? other._digits[i] : 0;
	}

	optimise(_digits);
	return *this;
}

big_int &big_int::operator^=(const big_int &other) & {
	size_t max_size = std::max(_digits.size(), other._digits.size());
	_digits.resize(max_size, 0);

	for (size_t i = 0; i < max_size; ++i) {
		_digits[i] ^= (i < other._digits.size()) ? other._digits[i] : 0;
	}

	optimise(_digits);
	return *this;
}

big_int &big_int::operator<<=(size_t shift) & {
	if (shift == 0 || is_zero(_digits)) 
        return *this;

    const size_t bits_per_digit = 8 * sizeof(unsigned int);
    const size_t digit_shift = shift / bits_per_digit;
    
    if (digit_shift > 0) 
    {
        _digits.insert(_digits.begin(), digit_shift, 0);
        shift %= bits_per_digit;
        if (shift == 0) return *this;
    }

    unsigned int carry = 0;
    for (auto& digit : _digits) 
    {
        unsigned int new_carry = digit >> (bits_per_digit - shift);
        digit = (digit << shift) | carry;
        carry = new_carry;
    }

    if (carry != 0)
        _digits.push_back(carry);

	optimise(_digits);
	return *this;
}

big_int &big_int::operator>>=(size_t shift) & {
	if (shift == 0 || is_zero(_digits)) 
        return *this;

    const size_t bits_per_digit = 8 * sizeof(unsigned int);
    
    const size_t digit_shift = shift / bits_per_digit;
    if (digit_shift > 0)
    {
        if (digit_shift >= _digits.size()) {
            //если сдвиг больше чем число разрядов то 0
            _digits = {0};
            _sign = true;
            return *this;
        }
        _digits.erase(_digits.begin(), _digits.begin() + digit_shift);
        shift %= bits_per_digit;
        if (shift == 0) return *this;
    }

    unsigned int carry = 0;
    for (auto it = _digits.begin(); it != _digits.end(); ++it) 
    {
        unsigned int new_carry = (*it << (bits_per_digit - shift));
        *it = (*it >> shift) | carry;
        carry = new_carry;
    }

	optimise(_digits);
	if (is_zero(_digits)) {
		_sign = true;
	}

	return *this;
}

big_int &big_int::plus_assign(const big_int &other, size_t shift) & {
	if (is_zero(other._digits)) {
        return *this;
    }

	if (_sign != other._sign) {
        big_int neg_other = other;
        neg_other._sign = _sign;
        return minus_assign(neg_other, shift);
    } else if (_sign == other._sign) {
		const size_t required_size = std::max(_digits.size(), other._digits.size() + shift);
		if (_digits.size() < required_size) {
			_digits.resize(required_size, 0);
		}

		unsigned long long carry = 0;
		for (size_t i = 0; i < required_size; ++i) {
			unsigned long long a = (i < _digits.size()) ? _digits[i] : 0;
			unsigned long long b = (i >= shift && (i - shift) < other._digits.size()) 
								? other._digits[i - shift] : 0;

			unsigned long long sum = a + b + carry;
			_digits[i] = static_cast<unsigned int>(sum % BASE);
			carry = sum / BASE;
		}

		if (carry > 0) {
			_digits.push_back(carry);
		}
	}

	optimise(_digits);
	if (is_zero(_digits)) {
		_sign = true;
	}

	return *this;
}

big_int &big_int::minus_assign(const big_int &other, size_t shift) & {
	if (is_zero(other._digits)) {
        return *this;
    }

	if (_sign != other._sign) {
        big_int neg_other = other;
        neg_other._sign = _sign;
        return plus_assign(neg_other, shift);
    } else if (_sign == other._sign) {
		big_int this_copy(*this);
		big_int other_copy(other);
		this_copy._sign = true;
		other_copy._sign = true;
		other_copy <<= shift;

		bool result_sign = _sign;
		if ((this_copy <=> other_copy) == std::strong_ordering::less) {
			result_sign = !result_sign;
			std::swap(this_copy._digits, other_copy._digits);
		}

		long long borrow = 0;
    	for (size_t i = 0; i < this_copy._digits.size(); ++i) {
			long long diff = this_copy._digits[i] - borrow;
			if (i < other_copy._digits.size()) {
				diff -= other_copy._digits[i];
			}

			if (diff < 0) {
				diff += BASE;
				borrow = 1;
			} else {
				borrow = 0;
			}
			this_copy._digits[i] = diff;
		}

		_digits = std::move(this_copy._digits);
		_sign = result_sign;
	}

	
	optimise(_digits);
	if (is_zero(_digits)) {
		_sign = true;
	}

	return *this;
}

big_int &big_int::operator*=(const big_int &other) & {
	return multiply_assign(other, big_int::multiplication_rule::trivial);
}

big_int &big_int::operator/=(const big_int &other) & {
	return divide_assign(other, big_int::division_rule::trivial);
}

std::string big_int::to_string() const {
	if (is_zero(_digits)) return "0";

	std::string res;
	big_int tmp = *this;
	tmp._sign = true;
	while (tmp) {
		auto val = tmp % 10;
		res += ('0' + val._digits[0]);
		tmp /= 10;
	}

	if (!_sign) {
		res += '-';
	}

	std::reverse(res.begin(), res.end());
	return res;
}

std::ostream &operator<<(std::ostream &stream, const big_int &value) {
	stream << value.to_string();
	return stream;
}

std::istream &operator>>(std::istream &stream, big_int &value) {
	std::string val;
	stream >> val;
	value = big_int(val);
	return stream;
}

bool big_int::operator==(const big_int &other) const noexcept {
	return std::strong_ordering::equal == (*this <=> other);
}

big_int::big_int(const std::vector<unsigned int, pp_allocator<unsigned int>> &digits, bool sign) : _digits(digits), _sign(sign) {
	if (_digits.empty()) {
		_digits.push_back(0);
	}
	optimise(_digits);
}

big_int::big_int(std::vector<unsigned int, pp_allocator<unsigned int>> &&digits, bool sign) noexcept : _digits(std::move(digits)), _sign(sign) {
	if (_digits.empty()) {
		_digits.push_back(0);
	}
	optimise(_digits);
}


big_int::big_int(const std::string &num, unsigned int radix, pp_allocator<unsigned int> allocator) : _sign(true), _digits(allocator) {
	if (radix > 36 || radix < 2)
        throw std::invalid_argument("Radix must be in interval [2, 36], but is " + std::to_string(radix));
	
	if (num.empty()) {
		_digits.push_back(0);
		return;
	}

	std::string num1 = num;
	bool is_neg = 0;
	if (num1[0] == '-' || num1[0] == '+') {
		is_neg = num1[0] == '-';
		num1 = num1.substr(1);
	} 

	while (num1.size() > 1 && num1[0] == '0') {
		num1 = num1.substr(1);
	}

	if (num1.empty()) {
		_digits.push_back(0);
		return;
	}

	for (char c: num1) {
		unsigned int digit;
		if (c >= '0' && c <= '9') {
			digit = c - '0';
		} else if (c >= 'A' && c <= 'Z') {
			digit = 10 + (c - 'A');
		} else if (c >= 'a' && c <= 'z') {
			digit = 10 + (c - 'a');
		} else {
			throw std::invalid_argument("Invalid character in number string");
		}

		if (digit >= radix) {
			throw std::invalid_argument("Digit exceeds radix");
		}

		*this *= radix; 
		*this += big_int(static_cast<long long>(digit), allocator);
	}

	_sign = !is_neg;
	if (is_zero(_digits)) {
		_sign = true;
	}
}

big_int::big_int(pp_allocator<unsigned int> allocator) : _digits({0}, allocator), _sign(true) {
}

big_int &big_int::multiply_assign(const big_int &other, big_int::multiplication_rule rule) & {
	//карацубу добавить
	if (is_zero(_digits)) {
        return *this;
    }

	if (is_zero(other._digits)) {
		_digits.clear();
		_digits.push_back(0);
		_sign = true;
		return *this;
	}

	const size_t new_size = _digits.size() + other._digits.size();
    std::vector<unsigned int, pp_allocator<unsigned int>> result(new_size, 0, _digits.get_allocator());

    for (size_t i = 0; i < _digits.size(); ++i) {
        uint64_t carry = 0;
        const uint64_t a = _digits[i];
        
        for (size_t j = 0; j < other._digits.size(); ++j) {
            uint64_t product = a * other._digits[j] + result[i+j] + carry;
            result[i+j] = static_cast<unsigned int>(product % BASE);
            carry = product / BASE;
        }

        for (size_t k = i + other._digits.size(); carry > 0 && k < new_size; ++k) {
            uint64_t sum = result[k] + carry;
            result[k] = sum % BASE;
            carry = sum / BASE;
        }
    }

    _digits = std::move(result);
    _sign = (_sign == other._sign);
	optimise(_digits);
	return *this;
}
//neno
big_int &big_int::divide_assign(const big_int &other, big_int::division_rule rule) & {
	if (is_zero(_digits)) return *this;
	if (is_zero(other._digits)) throw std::logic_error("Division by zero");

	big_int abs_this(*this);
	abs_this._sign = true;
	big_int abs_other(other);
	abs_other._sign = true;
	if (abs_this < abs_other) {
		_digits.clear();
		_digits.push_back(0);
		_sign = true;
		return *this;
	}

	std::vector<unsigned int, pp_allocator<unsigned int>> quotient(_digits.size(), 0, _digits.get_allocator());
	big_int remain(_digits.get_allocator());
	remain._digits.clear();
	remain._digits.push_back(0);
	for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; i--) {
		remain._digits.insert(remain._digits.begin(), _digits[i]);
		while (remain._digits.size() > 1 && remain._digits.back() == 0) {
			remain._digits.pop_back();
		}

		if (remain._digits.empty()) {
			remain._sign = true;
		}

		unsigned long long left = 0, q = 0, right = BASE;
		while (left <= right) {
			unsigned long long mid = left + (right - left) / 2;
			big_int temp = abs_other * big_int(static_cast<long long>(mid), _digits.get_allocator());
			if (remain >= temp) {
				q = mid;
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}

		if (q > 0) {
			big_int temp = abs_other * big_int(static_cast<long long>(q), _digits.get_allocator());
			remain -= temp;
		}
		quotient[i] = static_cast<unsigned int>(q);
	}

	_sign = (_sign == other._sign);
	_digits = std::move(quotient);
	optimise(_digits);
	return *this;
}
//neno
big_int &big_int::modulo_assign(const big_int &other, big_int::division_rule rule) & {
	if (is_zero(_digits)) return *this;
	if (is_zero(other._digits)) throw std::logic_error("Division by zero");

	big_int abs_this(*this);
	abs_this._sign = true;
	big_int abs_other(other);
	abs_other._sign = true;
	if (abs_this < abs_other) {
		_sign = true;
		return *this;
	}

	big_int remain(_digits.get_allocator());
	remain._digits.clear();
	remain._digits.push_back(0);
	for (int i = static_cast<int>(_digits.size()) - 1; i >= 0; --i) {
		remain._digits.insert(remain._digits.begin(), _digits[i]);
		while (remain._digits.size() > 1 && remain._digits.back() == 0) {
			remain._digits.pop_back();
		}

		if (remain._digits.empty()) {
			remain._sign = true;
		}

		unsigned long long left = 0, right = BASE;
		unsigned long long q = 0;
		while (left <= right) {
			unsigned long long mid = left + (right - left) / 2;
			big_int temp = abs_other * big_int(static_cast<long long>(mid), _digits.get_allocator());
			if (remain >= temp) {
				q = mid;
				left = mid + 1;
			} else {
				right = mid - 1;
			}
		}

		if (q > 0) {
			big_int temp = abs_other * big_int(static_cast<long long>(q), _digits.get_allocator());
			remain -= temp;
		}
	}

	_digits = std::move(remain._digits);
	_sign = true;
	optimise(_digits);
	return *this;
}

big_int operator""_bi(unsigned long long n) {
	return n;
}
