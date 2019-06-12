struct [[eosio::table]] comment {
    name owner;
    std::string message;
    std::string link;
    bool black = false;
    uint32_t report = 0;
    uint32_t revision = 0;
    uint64_t v1 = 0;

    uint64_t primary_key() const {
        return owner.value;
    }
    
    EOSLIB_SERIALIZE(
                     comment,
                     (owner)
                     (message)
                     (link)
                     (black)
                     (report)
                     (revision)
                     (v1)
                     )
};

typedef eosio::multi_index< "comment"_n, comment> comment_table;

struct [[eosio::table]] rcomment {
    name owner;
    uint32_t report = 0;
    bool black = false;
    uint64_t v1 = 0;

    uint64_t primary_key() const {
        return owner.value;
    }
    
    EOSLIB_SERIALIZE(
                     rcomment,
                     (owner)
                     (report)
                     (black)
                     (v1)
                     )
};

typedef eosio::multi_index< "rcomment"_n, rcomment> rcomment_table;
