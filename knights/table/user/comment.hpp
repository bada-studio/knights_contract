//@abi table comment i64
struct comment {
    name owner;
    std::string message;
    std::string link;
    bool black = false;
    uint32_t report = 0;
    uint32_t revision = 0;
    uint64_t v1 = 0;

    uint64_t primary_key() const {
        return owner;
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

typedef eosio::multi_index< N(comment), comment> comment_table;


//@abi table rcomment i64
struct rcomment {
    name owner;
    uint32_t report = 0;
    bool black = false;
    uint64_t v1 = 0;

    uint64_t primary_key() const {
        return owner;
    }
    
    EOSLIB_SERIALIZE(
                     rcomment,
                     (owner)
                     (report)
                     (black)
                     (v1)
                     )
};

typedef eosio::multi_index< N(rcomment), rcomment> rcomment_table;
