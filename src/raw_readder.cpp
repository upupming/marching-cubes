#include <QDebug>
#include <fstream>

#include "raw_reader.h"

RawReader::RawReader(std::string filename, const int Z, const int Y, const int X) {
    std::ifstream(filename, std::ios::in | std::ios::binary);
    // Copied from https://www.cplusplus.com/doc/tutorial/files/
    std::streampos size;

    std::ifstream file(filename, std::ios::in | std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        size = file.tellg();
        assert(size == Z * Y * X * sizeof(unsigned short));
        m_data = (unsigned short *)malloc(size);
        file.seekg(0, std::ios::beg);
        file.read((char *)m_data, size);
        file.close();

        qDebug() << "the entire file content is read";
    } else
        qDebug() << "Unable to open file";
}

RawReader::~RawReader() {
    free(m_data);
}

unsigned short *RawReader::data() const {
    return m_data;
}
