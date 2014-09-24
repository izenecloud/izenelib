/* vim: set tabstop=4 : */
// for convenient using...
//////////////////////////////////////////////////////////////////////////

// optional param:
//   FEBIRD_DATA_IO_LS_EX_T_PARAM
//   FEBIRD_DATA_IO_LS_EX_PARAM

#ifndef FEBIRD_DATA_IO_LS_EX_T_PARAM
#define FEBIRD_DATA_IO_LS_EX_T_PARAM
#endif

#ifndef FEBIRD_DATA_IO_LS_EX_PARAM
#define FEBIRD_DATA_IO_LS_EX_PARAM
#endif

#ifdef FEBIRD_LOAD_FUNCTION_NAME

// parameter for this file
//   FEBIRD_LOAD_FUNCTION_NAME
//   FEBIRD_DATA_INPUT_CLASS
//   FEBIRD_DATA_INPUT_LOAD_FROM(input, x)

template<class Object FEBIRD_DATA_IO_LS_EX_T_PARAM>
bool FEBIRD_LOAD_FUNCTION_NAME(Object& x FEBIRD_DATA_IO_LS_EX_PARAM,
							   const std::string& szFile,
							   bool fileMustExist=true,
							   bool printLog=true)
{
// 	std::unique_ptr<StatisticTime> pst;
// 	if (printLog)
// 	{
// 		std::ostringstream oss;
// 		oss << BOOST_STRINGIZE(FEBIRD_LOAD_FUNCTION_NAME) << ": \"" << szFile << "\"";
// 		pst.reset(new StatisticTime(oss.str(), std::cout));
// 	}
	FileStream file;
	file.open(szFile.c_str(), "rb");
	if (file.isOpen()) {
#ifdef FEBIRD_LOAD_SAVE_CONVENIENT_USE_STREAM_BUFFER
		setvbuf(file, 0, _IONBF, 0);
		SeekableStreamWrapper<FileStream*> fileWrapper(&file);
		SeekableInputBuffer sbuf(16*1024);
		sbuf.attach(&fileWrapper);
		FEBIRD_DATA_INPUT_CLASS<SeekableInputBuffer> input(&sbuf);
#else
		FEBIRD_DATA_INPUT_CLASS<FileStream*> input(&file);
#endif
		FEBIRD_DATA_INPUT_LOAD_FROM(input, x);
	}
	else
	{
		if (fileMustExist)
			FileStream::ThrowOpenFileException(szFile.c_str(), "rb");
		else {
			OpenFileException exp(szFile.c_str(), "mode=rb");
			if (printLog)
				printf("open file failed: %s\n", exp.what());
			return false;
		}
	}
	return true;
}

template<class Object FEBIRD_DATA_IO_LS_EX_T_PARAM>
void FEBIRD_LOAD_FUNCTION_NAME(Object& x FEBIRD_DATA_IO_LS_EX_PARAM,
							   FileStream& file,
							   const std::string& szTitle,
							   bool printLog=true)
{
// 	std::unique_ptr<StatisticTime> pst;
// 	if (printLog)
// 	{
// 		std::ostringstream oss;
// 		oss << BOOST_STRINGIZE(FEBIRD_LOAD_FUNCTION_NAME) << ", title: \"" << szTitle << "\"";
// 		pst.reset(new StatisticTime(oss.str(), std::cout));
// 	}
#ifdef FEBIRD_LOAD_SAVE_CONVENIENT_USE_STREAM_BUFFER
//	setvbuf(file, 0, _IONBF, 0);
	SeekableStreamWrapper<FileStream*> fileWrapper(&file);
	SeekableInputBuffer sbuf(16*1024);
	sbuf.attach(&fileWrapper);
	FEBIRD_DATA_INPUT_CLASS<SeekableInputBuffer> input(&sbuf);
#else
	FEBIRD_DATA_INPUT_CLASS<FileStream*> input(&file);
#endif
	FEBIRD_DATA_INPUT_LOAD_FROM(input, x);
}

template<class Object>
bool FEBIRD_LOAD_FUNCTION_NAME(pass_by_value<Object> x, 
							   const std::string& szFile,
							   bool fileMustExist=true,
							   bool printLog=true)
{
	return FEBIRD_LOAD_FUNCTION_NAME(x.get(), szFile, fileMustExist, printLog);
}

template<class Object>
void FEBIRD_LOAD_FUNCTION_NAME(pass_by_value<Object> x,
							   FileStream& file,
							   const std::string& szTitle,
							   bool printLog=true)
{
	FEBIRD_LOAD_FUNCTION_NAME(x.get(), file, szTitle, printLog);
}
#endif // FEBIRD_LOAD_FUNCTION_NAME

//////////////////////////////////////////////////////////////////////////

#ifdef FEBIRD_SAVE_FUNCTION_NAME

// parameter for this file
//   FEBIRD_SAVE_FUNCTION_NAME
//   FEBIRD_DATA_OUTPUT_CLASS
//   FEBIRD_DATA_OUTPUT_SAVE_TO(output, x)
template<class Object FEBIRD_DATA_IO_LS_EX_T_PARAM>
void FEBIRD_SAVE_FUNCTION_NAME(const Object& x FEBIRD_DATA_IO_LS_EX_PARAM,
							   const std::string& szFile,
							   bool printLog=true)
{
// 	std::unique_ptr<StatisticTime> pst;
// 	if (printLog)
// 	{
// 		std::ostringstream oss;
// 		oss << BOOST_STRINGIZE(FEBIRD_SAVE_FUNCTION_NAME) << ": \"" << szFile << "\"";
// 		pst.reset(new StatisticTime(oss.str(), std::cout));
// 	}
#ifdef FEBIRD_SAVE_FILE_OPEN_MODE
	const char* openMode = FEBIRD_SAVE_FILE_OPEN_MODE;
#undef FEBIRD_SAVE_FILE_OPEN_MODE
#else
	const char* openMode = "wb";
#endif
	FileStream file(szFile.c_str(), openMode);

#ifdef FEBIRD_LOAD_SAVE_CONVENIENT_USE_STREAM_BUFFER
	setvbuf(file, 0, _IONBF, 0);
	SeekableStreamWrapper<FileStream*> fileWrapper(&file);
	SeekableOutputBuffer sbuf(16*1024);
	sbuf.attach(&fileWrapper);
	FEBIRD_DATA_OUTPUT_CLASS<SeekableOutputBuffer> output(&sbuf);
#else
	FEBIRD_DATA_OUTPUT_CLASS<FileStream*> output(&file);
#endif
	FEBIRD_DATA_OUTPUT_SAVE_TO(output, x);
}

template<class Object FEBIRD_DATA_IO_LS_EX_T_PARAM>
void FEBIRD_SAVE_FUNCTION_NAME(const Object& x FEBIRD_DATA_IO_LS_EX_PARAM,
							   FileStream& file,
							   const std::string& szTitle,
							   bool printLog=true)
{
// 	std::unique_ptr<StatisticTime> pst;
// 	if (printLog)
// 	{
// 		std::ostringstream oss;
// 		oss << BOOST_STRINGIZE(FEBIRD_SAVE_FUNCTION_NAME) << ", title: \"" << szTitle << "\"";
// 		pst.reset(new StatisticTime(oss.str(), std::cout));
// 	}
#ifdef FEBIRD_LOAD_SAVE_CONVENIENT_USE_STREAM_BUFFER
//	setvbuf(file, 0, _IONBF, 0);
	SeekableStreamWrapper<FileStream*> fileWrapper(&file);
	SeekableOutputBuffer sbuf(16*1024);
	sbuf.attach(&fileWrapper);
	FEBIRD_DATA_OUTPUT_CLASS<SeekableOutputBuffer*> output(&sbuf);
#else
	FEBIRD_DATA_OUTPUT_CLASS<FileStream*> output(&file);
#endif
	FEBIRD_DATA_OUTPUT_SAVE_TO(output, x);
}

#endif // FEBIRD_SAVE_FUNCTION_NAME

#undef FEBIRD_DATA_IO_LS_EX_T_PARAM
#undef FEBIRD_DATA_IO_LS_EX_PARAM

#undef FEBIRD_LOAD_FUNCTION_NAME
#undef FEBIRD_SAVE_FUNCTION_NAME

#undef FEBIRD_DATA_INPUT_CLASS
#undef FEBIRD_DATA_INPUT_LOAD_FROM

#undef FEBIRD_DATA_OUTPUT_CLASS
#undef FEBIRD_DATA_OUTPUT_SAVE_TO
