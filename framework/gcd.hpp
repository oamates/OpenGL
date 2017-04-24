

template<typename uint_t> uint_t gcd(uint_t u, uint_t v)
{
    if (u == 0) return v;
    if (v == 0) return u;
 
    int shift;
    for (shift = 0; ((u | v) & 1) == 0; ++shift)
    {
         u >>= 1;
         v >>= 1;
    }
 
    while ((u & 1) == 0) u >>= 1;
 
    do
    {
        while ((v & 1) == 0) v >>= 1;
        if (u > v)
        {
            uint_t t = v; 
            v = u;
            u = t;
        }
        v -= u;
    }
    while (v != 0);

    return u << shift;
}

template<typename uint_t> bool coprime(uint_t u, uint_t v)
{
    if ((u == 0) || (v == 0) || (((u | v) & 1) == 0)) return false;
 
    while ((u & 1) == 0) u >>= 1;
 
    do {
        while ((v & 1) == 0) v >>= 1;
        if (u > v)
        {
            uint_t t = v; 
            v = u;
            u = t;
        }
        v -= u;
    }
    while (v);

    return u == 1;
}
