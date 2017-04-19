#pragma once

#include<gmpxx.h>

/** 
 * Contains all utilities the ease the use of the gmp library.
 */
namespace gmputils {

    /**
     * Calculates the floor of the rational represented by \p r.
     * Also converts the result from a mpz_class to an integer for which the type is
     * given by \p IntType.
     * @tparam IntType The integer type to which the result of the floor is converted.
     * @param r The rational to floor.
     * @returns The floor of \p r as an integer.
     */
    template<typename IntType>
        IntType floor_to_int(mpq_class r) {
            mpz_class quotient = r.get_num() / r.get_den();
            return static_cast<IntType>(quotient.get_si());
        }

    /**
     * Calculates the ceiling of the rational represented by \p r.
     * Also converts the result from mpz_class to an integer for which the tpye is given
     * by \p IntType.
     * @tparam IntType The intger type to which the result of the ceiling is converted.
     * @param r The rational to ceil.
     * @returns The ceiling of \p r as an integer.
     */
    template<typename IntType>
        IntType ceil_to_int(mpq_class r) {
            return floor_to_int<IntType>(r) + (r.get_num() % r.get_den() == mpz_class(0) ? 0 : 1);
        }

}
