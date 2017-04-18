#pragma once

#include<cmath>

namespace rat {

    template<typename T>
        T gcd(T n1, T n2) {
            T tmp;
            while (n2 != 0) {
                tmp = n1;
                n1 = n2;
                n2 = tmp % n2;
            }
            return n1;
        }

    template<typename T>
        struct Rational {
            public:
                explicit Rational(T num = 0) : num(num), denom(1) {}
                Rational(T num, T denom) : num(num), denom(denom) { this->simplify(); }
                T num, denom;

                Rational& operator+=(Rational const& rhs) {
                    this->num = this->num * rhs.denom + this->denom * rhs.num;
                    this->denom *= rhs.denom;
                    this->simplify();
                    return *this;
                }

                friend Rational operator-(Rational const& r) {
                    return Rational(-r.num, r.denom);
                }

                Rational& operator-=(Rational const& rhs) {
                    return *this += (-rhs);
                }

                Rational& operator*=(Rational const& rhs) {
                    this->num *= rhs.num;
                    this->denom *= rhs.denom;
                    this->simplify();
                    return *this;
                }

                Rational& operator/=(Rational const& rhs) {
                    return *this *= Rational(rhs.denom, rhs.num);
                }

                friend Rational operator+(Rational lhs, Rational const& rhs) {
                    return lhs += rhs;
                }

                friend Rational operator-(Rational lhs, Rational const& rhs) {
                    return lhs -= rhs;
                }

                friend Rational operator*(Rational lhs, Rational const& rhs) {
                    return lhs *= rhs;
                }

                friend Rational operator/(Rational lhs, Rational const& rhs) {
                    return lhs /= rhs;
                }

                friend Rational abs(Rational const& r) {
                    return Rational(std::abs(r.num), std::abs(r.denom));
                }


                friend inline bool operator<(Rational const& lhs, Rational const& rhs){ 
                    Rational lhs_copy(lhs);
                    Rational rhs_copy(rhs);
                    lhs_copy.num *= rhs.denom;
                    lhs_copy.denom *= rhs.denom;
                    rhs_copy.num *= lhs.denom;
                    rhs_copy.denom *= lhs.denom;
                    
                    return lhs_copy.num < rhs_copy.num;
                }

                friend inline bool operator>(Rational const& lhs, Rational const& rhs){ return rhs < lhs; }

                friend inline bool operator<=(Rational const& lhs, Rational const& rhs){ return !(lhs > rhs); }

                friend inline bool operator>=(Rational const& lhs, Rational const& rhs){ return !(lhs < rhs); }

                friend bool operator==(Rational const& lhs, Rational const& rhs){ 
                    Rational diff = lhs - rhs;
                    return diff.num == 0;
                }

                friend bool operator!=(Rational const& lhs, Rational const& rhs){ return !(lhs == rhs); }

                T floor_to_int() {
                    return this->num / this->denom;
                }

                T ceil_to_int() {
                    return this->num / this->denom + (this->num % this->denom == 0 ? 0 : 1);
                }

                template<typename Float>
                    Float to_float() {
                        return static_cast<Float>(this->num)/this->denom;
                    }

                Rational& simplify() {
                    if (this->num == 0) {
                        this->denom = 1;
                        return *this;
                    } else {
                        T gcd_num_denom = gcd(this->num, this->denom);
                        this->num /= gcd_num_denom;
                        this->denom /= gcd_num_denom;
                        if (denom < 0) {
                            this->num = -this->num;
                            this->denom = -this->denom;
                        }
                        return *this;
                    }
                }

        };
}
