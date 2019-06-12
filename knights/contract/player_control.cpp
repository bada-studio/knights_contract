#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))

#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}

#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21

static void decode(const uint8_t *input, uint32_t *output, uint32_t length) {
    for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
        output[i] = ((uint32_t)input[j]) | (((uint32_t)input[j + 1]) << 8) |
        (((uint32_t)input[j + 2]) << 16) | (((uint32_t)input[j + 3]) << 24);
    }
}

uint32_t player_control::shuffle_bit(uint32_t v, uint32_t n) {
    return ((v << n) | (v >> (32 - n))) & 0xFFFFFFFFU;
}

uint32_t player_control::seed_identity(name from) {
    return (eosio::tapos_block_prefix() ^ (from & 0xffffffff)) + (from >> 32);
}

uint32_t player_control::get_key(name from) {
    return (from >> 8) + 127;
}

uint32_t player_control::get_key2(name from) {
    return (uint32_t)(from >> 32) ^ (uint32_t)(from);
}

uint32_t player_control::get_checksum_key(name from) {
    return (from & 0xFFFFFFFF) ^ 0xAAAAAAAA;
}

uint32_t player_control::calculate_trx_hash(char* buf, int size) {
    uint8_t res[4] = {0, };

    for(int index = 0; index < size; index++) {
        res[index % 4] = (uint8_t)buf[index];
    }

    uint32_t state[4];
    state[0] = 0x67425301;
    state[1] = 0xefcdab89;
    state[2] = 0x98adbcfe;
    state[3] = 0x10432576;

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];
    decode((uint8_t*)buf, x, 64);

    FF (a, b, c, d, x[ 0], s11, 0xd76aa478);
    FF (d, a, b, c, x[ 1], s12, 0xe8c7b756);
    FF (c, d, a, b, x[ 2], s13, 0x242070db);
    FF (b, c, d, a, x[ 3], s14, 0xcc1bdeee);
    FF (a, b, c, d, x[ 4], s11, 0xf57c0faf);
    FF (d, a, b, c, x[ 5], s12, 0x477c862a);
    FF (c, d, a, b, x[ 6], s13, 0xa8304613);
    FF (b, c, d, a, x[ 7], s14, 0xfd469501);
    FF (a, b, c, d, x[ 8], s11, 0x698098d8);
    FF (d, a, b, c, x[ 9], s12, 0x8b44f7af);
    FF (c, d, a, b, x[10], s13, 0xffff5bb1);
    FF (b, c, d, a, x[11], s14, 0x895cd7be);
    FF (a, b, c, d, x[12], s11, 0x6b011922);
    FF (d, a, b, c, x[13], s12, 0xfd987193);
    FF (c, d, a, b, x[14], s13, 0xa967438e);
    FF (b, c, d, a, x[15], s14, 0x49b40821);

    HH (a, b, c, d, x[ 5], s31, 0xfffa3942);
    HH (d, a, b, c, x[ 8], s32, 0x871f6781);
    HH (c, d, a, b, x[11], s33, 0x6d9d6122);
    HH (b, c, d, a, x[14], s34, 0xf5de380c);
    HH (a, b, c, d, x[ 1], s31, 0xa4beea44);
    HH (d, a, b, c, x[ 4], s32, 0x4bdaecf9);
    HH (c, d, a, b, x[ 7], s33, 0xf6bb4b60);
    HH (b, c, d, a, x[10], s34, 0xbecbfb70);
    HH (a, b, c, d, x[13], s31, 0x289b7ec6);
    HH (d, a, b, c, x[ 0], s32, 0xeaa127fa);
    HH (c, d, a, b, x[ 3], s33, 0xd4f30e85);
    HH (b, c, d, a, x[ 6], s34,  0x4881d05);
    HH (a, b, c, d, x[ 9], s31, 0xd9d4d039);
    HH (d, a, b, c, x[12], s32, 0xe69db9e5);
    HH (c, d, a, b, x[15], s33, 0x1f27caf8);
    HH (b, c, d, a, x[ 2], s34, 0xc46ac565);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    return (state[0] ^ state[1] ^ state[2] ^ state[3]);
}

uint32_t player_control::calculate_trx_hash2() {
    auto v = (uint32_t)current_time();
    char buffer[64] = {0, };

    for (int index = 0; index < 64; index++) {
        int n = (index % 32);
        buffer[index] = ROTATELEFT(v, n);
    }

    return calculate_trx_hash(buffer, 64);
}

void player_control::check_blacklist(name from) {
    if (from == "valuenetwork"_n || 
        from == "ramcollector"_n || 
        from == "mrnumberzero"_n || 
        from == "siuhangmeiyu"_n || 
        from == "amazinggamer"_n || 
        from == "mantikmantik"_n || 
        from == "meiyusiuhang"_n ||
        from == "gameplayer11"_n ||
        from == "gameplayer12"_n ||
        from == "gameplayer13"_n ||
        from == "gameplayer14"_n ||
        from == "gameplayer15"_n ||
        from == "eos4chatting"_n ||
        from == "gi4temzqhege"_n ||
        from == "messcomposer"_n ||
        from == "pvtmessenger"_n ||
        from == "eospromoter1"_n ||

        from == "burjuiburjui"_n ||
        from == "cashnotfound"_n ||
        from == "pimpmyknight"_n ||
        from == "singularityx"_n ||
        from == "chessxplayer"_n ||
        from == "margincallxx"_n ||
        from == "eosexcalibur"_n ||
        from == "kingofrandom"_n ||
        from == "dadarkknight"_n ||
        from == "triplesevenx"_n ||
        from == "xluckygamerx"_n ||
        from == "gamingmaster"_n ||
        from == "gaminglegend"_n ||
        from == "eostokenizer"_n ||

        from == "aidanjackso2"_n ||

        from == "v.io"_n ||

        from == "xtigerlegend"_n ||
        from == "xtigershadow"_n ||
        from == "xtigergalaxy"_n ||
        from == "xtigermoment"_n ||
        from == "xtigerrrrrrr"_n ||
        from == "xtiger.game"_n ||
        from == "tiger.x"_n ||
        from == "oi.io"_n ||
        from == "greedysogood"_n ||
        
        // buy bot
        from == "g5rzmqruqygd"_n 
    ) {
        assert_true(false, "blacklist rejected");
    }
}
