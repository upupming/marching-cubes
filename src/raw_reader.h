#pragma once
#include <string>
class RawReader {
   public:
    RawReader(const std::string filename, const int Z, const int Y, const int X);
    unsigned short* data() const;
    ~RawReader();

   private:
    unsigned short* m_data;
};
