
#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <memory>
#include <string>
#include <unordered_map>
#include <list>
#include <memory>
#include <variant>

class Arg;
class StringArg;
class IntArg;
class FloatArg;
class SizeArg;

enum class ArgType {
    kNone, kString, kStringMap, kInt, kFloat, kSize
};

class Arg {
protected:
    ArgType type_ = ArgType::kNone;
    bool is_set_ = false;
public:
    Arg() = default;
    bool IsSet() const { return is_set_; }
    virtual int Add(std::string arg) { throw 1; };
    virtual void AddStringMapVal(std::string val, int id) { throw 1; };
    virtual std::string &GetStringOpt() { throw 1; };
    virtual std::list<std::string> &GetStringOpts() { throw 1; };
    virtual int GetIntOpt() { throw 1; };
    virtual std::list<int> &GetIntOpts() { throw 1; };
    virtual float GetFloatOpt() { throw 1; };
    virtual std::list<float> &GetFloatOpts() { throw 1; };
    virtual size_t GetSizeOpt() { throw 1; };
    virtual std::list<size_t> &GetSizeOpts() { throw 1; };
};

typedef std::unique_ptr<Arg> ArgPtr;

class StringArg : public Arg {
private:
    std::list<std::string> args_;

public:
    StringArg() { type_ = ArgType::kString; };
    int Add(std::string arg) override {
        args_.emplace_back(arg);
        is_set_ = true;
        return 1;
    }
    std::string &GetStringOpt() override { return args_.back(); }
    std::list<std::string> &GetStringOpts() override {return args_; }
};

class StringMapArg : public Arg {
private:
    std::unordered_map<std::string, int> arg_map_;
    std::list<int> args_;

public:
    StringMapArg() { type_ = ArgType::kStringMap; };
    int Add(std::string arg) override {
        args_.emplace_back(arg_map_[arg]);
        is_set_ = true;
        return 1;
    }
    void AddStringMapVal(std::string val, int id) override { arg_map_[val] = id; }
    int GetIntOpt() override { return args_.back(); };
    std::list<int> &GetIntOpts() override { return args_; };
};

class IntArg : public Arg {
private:
    std::list<int> args_;

public:
    IntArg() { type_ = ArgType::kInt; };
    int Add(std::string arg) override {
        args_.emplace_back(std::stoi(arg));
        is_set_ = true;
        return 1;
    }
    int GetIntOpt() override { return args_.back(); };
    std::list<int> &GetIntOpts() override { return args_; };
};

class FloatArg : public Arg {
private:
    std::list<float> args_;

public:
    FloatArg() { type_ = ArgType::kFloat; };
    int Add(std::string arg) override {
        args_.emplace_back(std::stof(arg));
        is_set_ = true;
        return 1;
    }
    float GetFloatOpt() override { return args_.back(); };
    std::list<float> &GetFloatOpts() override { return args_; };
};

class SizeArg : public Arg {
private:
    std::list<size_t> args_;
    size_t ToSize(std::string arg) {
        char c = arg[arg.length() - 1];
        if ((c == 'k') || (c == 'K')) {
            return std::stoul(arg) * (1ul<<10);
        }
        else if ((c == 'm') || (c == 'M')) {
            return std::stoul(arg) * (1ul<<20);
        }
        else if ((c == 'g') || (c == 'G')) {
            return std::stoul(arg) * (1ul<<30);
        }
        return std::stoul(arg);
    }
public:
    SizeArg() { type_ = ArgType::kSize; };
    int Add(std::string arg) override {
        args_.emplace_back(ToSize(arg));
        is_set_ = true;
        return 1;
    }
    size_t GetSizeOpt() override { return args_.back(); };
    std::list<size_t> &GetSizeOpts() override { return args_; };
};

class ArgFactory {
public:
    static ArgPtr Get(ArgType type) {
        switch (type) {
            case ArgType::kNone: {
                return std::make_unique<Arg>();
            }
            case ArgType::kString: {
                return std::make_unique<StringArg>();
            }
            case ArgType::kStringMap: {
                return std::make_unique<StringMapArg>();
            }
            case ArgType::kInt: {
                return std::make_unique<IntArg>();
            }
            case ArgType::kFloat: {
                return std::make_unique<FloatArg>();
            }
            case ArgType::kSize: {
                return std::make_unique<SizeArg>();
            }
        }
    }
};

class ArgMap {
protected:
    std::unordered_map<std::string, ArgPtr> args_;

    void AddOpt(std::string opt, ArgType type = ArgType::kNone) {
        args_.emplace(opt, std::move(ArgFactory::Get(type)));
    }

    void AddStringMapVal(std::string opt, std::string val, int id) {
        args_[opt]->AddStringMapVal(val, id);
    }

    void ArgIter(int argc, char **argv) {
        for(int i = 1; i < argc; ++i) {
            if(argv[i][0] != '-') {
                ArgPtr &arg = args_[""];
                i += arg->Add(argv[i+1]);
                continue;
            }
            ArgPtr &arg = args_[argv[i]];
            i += arg->Add(argv[i+1]);
        }
    }

    virtual void VerifyArgs(void) = 0;

public:
    ArgMap() = default;

    bool OptIsSet(std::string opt) {
        return args_[opt]->IsSet();
    }

    std::string &GetStringOpt(std::string opt) { return args_[opt]->GetStringOpt(); }
    std::list<std::string> &GetStringOpts(std::string opt) { return args_[opt]->GetStringOpts(); }
    int GetIntOpt(std::string opt) { return args_[opt]->GetIntOpt(); }
    std::list<int> &GetIntOpts(std::string opt) { args_[opt]->GetIntOpts(); }
    float GetFloatOpt(std::string opt) { return args_[opt]->GetFloatOpt(); }
    std::list<float> &GetFloatOpts(std::string opt) { return args_[opt]->GetFloatOpts(); }
    size_t GetSizeOpt(std::string opt) { return args_[opt]->GetSizeOpt(); }
    std::list<size_t> &GetSizeOpts(std::string opt) { return args_[opt]->GetSizeOpts(); }

    virtual void Usage(void) = 0;
};

#endif