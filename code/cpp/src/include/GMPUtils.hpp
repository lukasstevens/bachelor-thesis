#pragma once

#include<gmpxx.h>

namespace gmputils {

    template<typename IntType>
        IntType floor_to_int(mpq_class r) {
            mpz_class quotient = r.get_num() / r.get_den();
            return static_cast<IntType>(quotient.get_si());
        }

    template<typename IntType>
        IntType ceil_to_int(mpq_class r) {
            return floor_to_int<IntType>(r) + (r.get_num() % r.get_den() == mpz_class(0) ? 0 : 1);
        }

}
