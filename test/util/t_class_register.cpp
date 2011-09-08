#include <util/class_register.h>

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include <string>
#include <iostream>

using namespace std;

class Mapper {
public:
    Mapper() {}
    virtual ~Mapper() {}

    virtual std::string GetMapperName() const = 0;
};

CLASS_REGISTER_DEFINE_REGISTRY(mapper_register, Mapper);

#define REGISTER_MAPPER(mapper_name)                            \
  CLASS_REGISTER_OBJECT_CREATOR(                                \
      mapper_register, Mapper, #mapper_name, mapper_name)

#define CREATE_MAPPER(mapper_name_as_string)                            \
  CLASS_REGISTER_CREATE_OBJECT(mapper_register, mapper_name_as_string)

CLASS_REGISTER_DEFINE_REGISTRY(second_mapper_register, Mapper);

#define REGISTER_SECONDARY_MAPPER(mapper_name)                          \
  CLASS_REGISTER_OBJECT_CREATOR(                                        \
      second_mapper_register, Mapper, #mapper_name, mapper_name)

#define CREATE_SECONDARY_MAPPER(mapper_name_as_string)  \
  CLASS_REGISTER_CREATE_OBJECT(second_mapper_register,  \
                               mapper_name_as_string)


class Reducer {
public:
    Reducer() {}
    virtual ~Reducer() {}

    virtual std::string GetReducerName() const = 0;
};

CLASS_REGISTER_DEFINE_REGISTRY(reducer_register, Reducer);

#define REGISTER_REDUCER(reducer_name)                          \
  CLASS_REGISTER_OBJECT_CREATOR(                                \
      reducer_register, Reducer, #reducer_name, reducer_name)

#define CREATE_REDUCER(reducer_name_as_string)                          \
  CLASS_REGISTER_CREATE_OBJECT(reducer_register, reducer_name_as_string)



class FileImpl {
public:
    FileImpl() {}
    virtual ~FileImpl() {}

    virtual std::string GetFileImplName() const = 0;
};

CLASS_REGISTER_DEFINE_REGISTRY(file_impl_register, FileImpl);

#define REGISTER_DEFAULT_FILE_IMPL(file_impl_name)      \
  CLASS_REGISTER_DEFAULT_OBJECT_CREATOR(                \
      file_impl_register, FileImpl, file_impl_name)

#define REGISTER_FILE_IMPL(path_prefix_as_string, file_impl_name)       \
  CLASS_REGISTER_OBJECT_CREATOR(                                        \
      file_impl_register, FileImpl, path_prefix_as_string, file_impl_name)

#define CREATE_FILE_IMPL(path_prefix_as_string)                         \
  CLASS_REGISTER_CREATE_OBJECT(file_impl_register, path_prefix_as_string)


CLASS_REGISTER_IMPLEMENT_REGISTRY(mapper_register, Mapper);
CLASS_REGISTER_IMPLEMENT_REGISTRY(second_mapper_register, Mapper);
CLASS_REGISTER_IMPLEMENT_REGISTRY(reducer_register, Reducer);
CLASS_REGISTER_IMPLEMENT_REGISTRY(file_impl_register, FileImpl);


class HelloMapper : public Mapper {
  virtual std::string GetMapperName() const {
    return "HelloMapper";
  }
};
REGISTER_MAPPER(HelloMapper);

class WorldMapper : public Mapper {
  virtual std::string GetMapperName() const {
    return "WorldMapper";
  }
};
REGISTER_MAPPER(WorldMapper);

class SecondaryMapper : public Mapper {
  virtual std::string GetMapperName() const {
    return "SecondaryMapper";
  }
};
REGISTER_SECONDARY_MAPPER(SecondaryMapper);


class HelloReducer : public Reducer {
  virtual std::string GetReducerName() const {
    return "HelloReducer";
  }
};
REGISTER_REDUCER(HelloReducer);

class WorldReducer : public Reducer {
  virtual std::string GetReducerName() const {
    return "WorldReducer";
  }
};
REGISTER_REDUCER(WorldReducer);


class LocalFileImpl : public FileImpl {
  virtual std::string GetFileImplName() const {
    return "LocalFileImpl";
  }
};
REGISTER_DEFAULT_FILE_IMPL(LocalFileImpl);
REGISTER_FILE_IMPL("/local", LocalFileImpl);

class MemFileImpl : public FileImpl {
  virtual std::string GetFileImplName() const {
    return "MemFileImpl";
  }
};
REGISTER_FILE_IMPL("/mem", MemFileImpl);

class NetworkFileImpl : public FileImpl {
  virtual std::string GetFileImplName() const {
    return "NetworkFileImpl";
  }
};
REGISTER_FILE_IMPL("/nfs", NetworkFileImpl);

BOOST_AUTO_TEST_CASE(Class_register_test_CreateMapper)
{
    boost::scoped_ptr<Mapper> mapper;
    mapper.reset(CREATE_MAPPER(""));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloMapper "));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloWorldMapper"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloReducer"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("WorldReducer"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("SecondaryMapper"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_MAPPER("HelloMapper"));
    BOOST_CHECK(mapper.get() != NULL);
    BOOST_CHECK_EQUAL("HelloMapper", mapper->GetMapperName());

    mapper.reset(CREATE_MAPPER("WorldMapper"));
    BOOST_CHECK(mapper.get() != NULL);
    BOOST_CHECK_EQUAL("WorldMapper", mapper->GetMapperName());
}

BOOST_AUTO_TEST_CASE(Class_register_test_CreateSecondaryMapper) 
{
    boost::scoped_ptr<Mapper> mapper;
    mapper.reset(CREATE_SECONDARY_MAPPER(""));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("SecondaryMapper "));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("HelloWorldMapper"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("HelloReducer"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("WorldReducer"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("HelloMapper"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("WorldMapper"));
    BOOST_CHECK(mapper.get() == NULL);

    mapper.reset(CREATE_SECONDARY_MAPPER("SecondaryMapper"));
    BOOST_CHECK(mapper.get() != NULL);
    BOOST_CHECK_EQUAL("SecondaryMapper", mapper->GetMapperName());
}

BOOST_AUTO_TEST_CASE(Class_register_test_CreateReducer) 
{
    boost::scoped_ptr<Reducer> reducer;
    reducer.reset(CREATE_REDUCER(""));
    BOOST_CHECK(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloReducer "));
    BOOST_CHECK(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloWorldReducer"));
    BOOST_CHECK(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloMapper"));
    BOOST_CHECK(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("WorldMapper"));
    BOOST_CHECK(reducer.get() == NULL);

    reducer.reset(CREATE_REDUCER("HelloReducer"));
    BOOST_CHECK(reducer.get() != NULL);
    BOOST_CHECK_EQUAL("HelloReducer", reducer->GetReducerName());

    reducer.reset(CREATE_REDUCER("WorldReducer"));
    BOOST_CHECK(reducer.get() != NULL);
    BOOST_CHECK_EQUAL("WorldReducer", reducer->GetReducerName());
}

BOOST_AUTO_TEST_CASE(Class_register_test_CreateFileImpl) 
{
    boost::scoped_ptr<FileImpl> file_impl;
    file_impl.reset(CREATE_FILE_IMPL("/mem"));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("MemFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/nfs"));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("NetworkFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/local"));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("LocalFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/"));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("LocalFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL(""));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("LocalFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/mem2"));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("LocalFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/mem/"));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("LocalFileImpl", file_impl->GetFileImplName());

    file_impl.reset(CREATE_FILE_IMPL("/nfs2/"));
    BOOST_CHECK(file_impl.get() != NULL);
    BOOST_CHECK_EQUAL("LocalFileImpl", file_impl->GetFileImplName());
}


