#ifndef COVER_GENERATOR_H
#define COVER_GENERATOR_H

class CoverGenerator {
public:

    CoverGenerator(uint_fast16_t size) {
        _size = size;
        _seed = 0;
        _current = new uint_fast8_t[size];
    }

    inline uint_fast16_t next(uint_fast8_t* buffer, uint_fast16_t amount) {
        uint_fast16_t count = 0;
            
        for (;;) {
            if (count == amount) {
                return count;
            }
            if (_seed > _size) {
                return count;
            }
            
            uint_fast16_t index = _size - 1;
            while (index > 0 && !(_current[index] == 1 && _current[index-1] == 0)) {
                index -= 1;
            }

            uint_fast16_t amount = 0;
            
            if (index != 0) {
                _current[index - 1] = 1;
                for (uint_fast16_t i = index + 1; i < _size; ++i) {
                    amount += _current[i];
                }
            } else {
                _seed += 1;
                if (_seed > _size) {
                    return count;
                }
                
                amount = _seed;
            }

            for (uint_fast16_t i = 0; i < (_size - index + 1); ++i) {
                _current[_size - i] = i <= amount ? 1 : 0;
            }

            for (uint_fast16_t i = 0; i < _size; ++i) {
                buffer[_size * count + i] = _current[i];
            }
            count += 1;
        }
    }

    virtual ~CoverGenerator() {
        delete [] _current;
    }

private:
    uint_fast16_t _size;
    uint_fast16_t _seed;
    uint_fast8_t* _current;
};

#endif // COVER_GENERATOR_H
