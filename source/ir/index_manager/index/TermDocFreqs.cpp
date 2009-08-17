#include <ir/index_manager/index/TermDocFreqs.h>
#include <ir/index_manager/index/InputDescriptor.h>
#include <ir/index_manager/index/TermReader.h>
#include <ir/index_manager/index/Term.h>


using namespace izenelib::ir::indexmanager;

TermDocFreqs::TermDocFreqs(void)
        :pPosting(NULL)
        ,pPostingBuffer(0)
        ,nBufferSize(0)
        ,nFreqStart(0)
        ,nTotalDecodedCount(0)
        ,nCurDecodedCount(0)
        ,nCurrentPosting(-1)
        ,pTermReader(NULL)
        ,pInputDescriptor(NULL)
{
}

TermDocFreqs::TermDocFreqs(TermReader* pReader, InputDescriptor* pInputDescriptor_, TermInfo& ti)
        :pPostingBuffer(NULL)
        ,nBufferSize(0)
        ,nFreqStart(0)
        ,nTotalDecodedCount(0)
        ,nCurDecodedCount(0)
        ,nCurrentPosting(-1)
        ,pTermReader(pReader)
        ,pInputDescriptor(pInputDescriptor_)
{
    termInfo.set(ti.docFreq(),ti.docPointer());
    pInputDescriptor->getDPostingInput()->seek(ti.docPointer());
    pPosting = new OnDiskPosting(pInputDescriptor,ti.docPointer());
}

TermDocFreqs::TermDocFreqs(TermReader* pReader, Posting* pposting, TermInfo& ti)
        :pPosting(pposting->clone())
        ,pPostingBuffer(NULL)
        ,nBufferSize(0)
        ,nFreqStart(0)
        ,nTotalDecodedCount(0)
        ,nCurDecodedCount(0)
        ,nCurrentPosting(-1)
        ,pTermReader(pReader)
        ,pInputDescriptor(NULL)
{
    termInfo.set(ti.docFreq(),ti.docPointer());
}

TermDocFreqs::~TermDocFreqs(void)
{
    close();
    if (pPostingBuffer)
    {
        delete[] pPostingBuffer;
        pPostingBuffer = NULL;
    }
    nBufferSize = 0;
}

freq_t TermDocFreqs::docFreq()
{
    return termInfo.docFreq();
}

int64_t TermDocFreqs::getCTF()
{
    return pPosting->getCTF();
}

docid_t TermDocFreqs::doc()
{
    return pPostingBuffer[nCurrentPosting];
}

count_t TermDocFreqs::freq()
{
    return pPostingBuffer[nFreqStart + nCurrentPosting];
}

freq_t TermDocFreqs::docLength()
{
    return pPostingBuffer[nFreqStart + sizeof(count_t) + nCurrentPosting];
}

bool TermDocFreqs::next()
{
    nCurrentPosting ++;
    if (nCurrentPosting >= nCurDecodedCount)
    {
        if (!decode())
            return false;
        return true;
    }
    return true;
}
count_t TermDocFreqs::next(docid_t*& docs, count_t*& freqs)
{
    if ((nCurrentPosting < 0) || (nCurrentPosting >= nCurDecodedCount) )
    {
        if (!decode())
            return false;
    }
    docs = &pPostingBuffer[nCurrentPosting];
    freqs = &pPostingBuffer[nFreqStart + nCurrentPosting];
    int32_t c = nCurDecodedCount - nCurrentPosting;
    nCurDecodedCount = nCurrentPosting = 0;
    return c;
}

void TermDocFreqs::close()
{
    if (pInputDescriptor)
    {
        delete pInputDescriptor;
        pInputDescriptor = NULL;
        pTermReader = NULL;
    }
    if (pPosting)
    {
        delete pPosting;
        pPosting = NULL;
    }
}
bool TermDocFreqs::decode()
{
    count_t df = termInfo.docFreq();
    if ((count_t)nTotalDecodedCount == df)
    {
        return false;
    }

    if (!pPostingBuffer)
        createBuffer();

    nCurrentPosting = 0;
    nCurDecodedCount = pPosting->decodeNext(pPostingBuffer,nBufferSize);
    if (nCurDecodedCount <= 0)
        return false;
    nTotalDecodedCount += nCurDecodedCount;

    return true;
}
void TermDocFreqs::createBuffer()
{
    size_t bufSize = TermDocFreqs::DEFAULT_BUFFERSIZE;
    if (pPostingBuffer == NULL)
    {
        nBufferSize = bufSize;
        nFreqStart = nBufferSize/2;
        pPostingBuffer = new uint32_t[nBufferSize];
    }
}

