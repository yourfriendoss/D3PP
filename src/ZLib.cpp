//
// Created by unknown on 4/2/21.
//

#include "ZLib.h"

int GZIP::GZip_Decompress(unsigned char *output, int outputLen, unsigned char *input, int inputLen) {
    z_stream asdf;
    asdf.zalloc = Z_NULL;
    asdf.zfree = Z_NULL;
    asdf.opaque = Z_NULL;
    asdf.avail_in = inputLen;
    asdf.avail_out = outputLen;
    asdf.next_in = input;
    asdf.next_out = output;

    int infResult = inflateInit2_(&asdf, 15+16, zlibVersion(), sizeof(z_stream));

    if (infResult != Z_OK) {
        return infResult;
    }

    int aResult = inflate(&asdf, Z_NO_FLUSH);

    if (aResult != Z_STREAM_END && aResult != Z_OK) {
        return aResult;
    }

    inflateEnd(&asdf);
    return (int)asdf.total_out;
}

int GZIP::GZip_Compress(unsigned char *output, int outputLen, unsigned char *input, int inputLen) {
    z_stream asdf;
    asdf.zalloc = Z_NULL;
    asdf.zfree = Z_NULL;
    asdf.opaque = Z_NULL;
    asdf.avail_in = inputLen;
    asdf.avail_out = outputLen;
    asdf.next_in = input;
    asdf.next_out = output;

    int infResult = deflateInit2_(&asdf, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY, zlibVersion(), sizeof(z_stream));

    if (infResult != Z_OK) {
        return infResult;
    }

    int aResult = deflate(&asdf, Z_FINISH);

    if (aResult != Z_STREAM_END && aResult != Z_OK) {
        return aResult;
    }

    deflateEnd(&asdf);
    return (int)asdf.total_out;
}

int GZIP::GZip_CompressBound(int inputLen) {
    return compressBound(inputLen);
}

bool GZIP::GZip_CompressToFile(unsigned char *input, int inputLen, std::string filename) {
    unsigned char* compressed;
    compressed = new unsigned char[inputLen];
    int compResult = GZip_Compress(compressed, inputLen, input, inputLen);

    if (compResult == -1)
        return false;

    ofstream wf(filename, ios::out | ios::binary);
    wf.write((char *)compressed, compResult);
    wf.close();

    return true;
}

int GZIP::GZip_DecompressFromFile(unsigned char *output, int outputLen, std::string filename) {
    int fileSize = Utils::FileSize(filename);

    ifstream of(filename, ios::in | ios::binary);

    char* data = new char[fileSize];
    of.read(data, fileSize);
    of.close();

    int decompResult = GZip_Decompress(output, outputLen, (unsigned char*)data, fileSize);

    if (decompResult == -1)
        return 0;

    return decompResult;
}
