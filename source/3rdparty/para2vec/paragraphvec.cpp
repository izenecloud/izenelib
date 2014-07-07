#include "paragraphvec.h"

extern void setParameter(const ModelArgument& ma);
extern bool initML();
extern void TrainMLModel();
extern std::vector<real> Predict(const std::string& para);

ParagraphVector::ParagraphVector(const ModelArgument& ma)
{
    setParameter(ma);
    initML();
}

ParagraphVector::~ParagraphVector()
{

}

void ParagraphVector::TrainModel()
{
    TrainMLModel();
}

std::vector<real> ParagraphVector::PredictPara( const std::string& para )
{
    return Predict(para);
}
