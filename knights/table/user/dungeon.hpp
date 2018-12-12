#pragma once

struct dgknight {
    uint8_t type = 0;
    uint32_t attack = 0;
    uint32_t defense = 0;
    uint32_t hp = 0;
    std::vector<kntskill> skills;
};

struct dgdata {
    uint16_t code = 0;
    uint32_t seed = 0;
    uint64_t v1 = 0;
    std::vector<dgknight> knts;
};

struct dgrecords {
    uint32_t id = 0;
    uint16_t code = 0;
    uint32_t at = 0;
    uint32_t win = 0;
    uint32_t lose = 0;
    uint64_t v1 = 0;
};

struct dgticket {
    uint16_t code = 0;
    uint16_t count = 0;
    uint32_t free_at = 0;
    uint8_t free_count = 0;
    uint8_t v1 = 0;
    uint16_t v2 = 0;
    uint32_t v3 = 0;

    uint32_t get_total_count() const {
        return count + free_at;
    }

    void reduce_count(uint32_t amount) {
        if (free_count >= amount) {
            free_count -= amount;
            return;
        }

        amount -= free_count;
        free_count = 0;

        if (count >= amount) {
            count -= amount;
            return;
        }

        count = 0;
    }
};

//@abi table dungeons i64
struct dungeons {
    name owner;
    std::vector<dgticket> tickets;
    std::vector<dgdata> rows;
    std::vector<dgrecords> records;
    uint64_t v1 = 0;
    uint64_t v2 = 0;

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

    int find_record(int code) const {
        for (int index = 0; index < records.size(); index++) {
            if (records[index].code == code) {
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
            (records)
            (v1)
            (v2)
    )
};

typedef eosio::multi_index< N(dungeons), dungeons> dungeons_table;
