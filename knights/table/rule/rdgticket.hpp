#pragma once

//@abi table rdgticket i64
struct rdgticket {
    uint64_t code = 0;
    uint16_t mat1 = 0;
    uint8_t cnt1 = 0;
    uint16_t mat2 = 0;
    uint8_t cnt2 = 0;
    uint16_t mat3 = 0;
    uint8_t cnt3 = 0;

    rdgticket() {
    }

    uint64_t primary_key() const {
        return code;
    }

    EOSLIB_SERIALIZE(
            rdgticket,
            (code)
            (mat1)
            (cnt1)
            (mat2)
            (cnt2)
            (mat3)
            (cnt3)
    )
};

typedef eosio::multi_index< N(rdgticket), rdgticket> rdgticket_table;
