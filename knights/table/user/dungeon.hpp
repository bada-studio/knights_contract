#pragma once

struct dgknight {
    uint8_t type;
    uint32_t attack;
    uint32_t defense;
    uint32_t hp;
    std::vector<kntskill> skills;
};

struct dgdata {
    uint16_t code = 0;
    uint32_t seed = 0;
    uint32_t open = 0;
    uint64_t v1 = 0;
    uint64_t v2 = 0;
    std::vector<dgknight> knts;
};

struct dgrecord {
    uint32_t win;
    uint32_t lose;
};

struct dgticket {
    uint16_t code;
    uint16_t count;
};

//@abi table dungeons i64
struct dungeons {
    name owner;
    dgrecord records;
    std::vector<dgticket> tickets;
    std::vector<dgdata> rows;

    uint64_t primary_key() const {
        return owner;
    }

    int find_ticket(int code) const {
        for (int index = 0; index < tickets.size(); index++) {
            if (tickets[index].code == code) {
                return index;
            }
        }
        return -1;
    }

    int find_data(int code) const {
        for (int index = 0; index < rows.size(); index++) {
            if (rows[index].code == code) {
                return index;
            }
        }
        return -1;
    }

    EOSLIB_SERIALIZE(
            dungeons,
            (owner)
            (tickets)
            (rows)
    )
};

typedef eosio::multi_index< N(dungeons), dungeons> dungeons_table;
