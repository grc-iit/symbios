
#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <string>
#include <unordered_map>
#include <list>
#include <memory>
#include <variant>

enum class ArgType {
    kNone, kString, kInt, kFloat, kSize
};



class Arg {
private:
    ArgType type_ = ArgType::kNone;
    bool has_input_;
    bool is_set_ = false;

    //Would've used std::variant, but it requires C++17, which may be too new...
    std::list<std::string> args_str_;
    std::list<int> args_int_;
    std::list<float> args_float_;
    std::list<size_t> args_size_;

    size_t ToSize(std::string arg) {
        if ((arg[arg.length() - 1] == 'k') || (arg[arg.length() - 1] == 'K')) {
            return std::stoul(arg) * (1ul<<10);
        }
        else if ((arg[arg.length() - 1] == 'm') || (arg[arg.length() - 1] == 'M')) {
            return std::stoul(arg) * (1ul<<20);
        }
        else if ((arg[arg.length() - 1] == 'g') || (arg[arg.length() - 1] == 'G')) {
            return std::stoul(arg) * (1ul<<30);
        }

        return std::stoul(arg);
    }

public:
    Arg() = default;
    Arg(ArgType type) : type_(type) {}
    int Add(std::string arg) {
        switch (type_) {
            case ArgType::kString: {
                args_str_.emplace_back(arg);
                break;
            }
            case ArgType::kInt: {
                args_int_.emplace_back(std::stoi(arg));
                break;
            }
            case ArgType::kFloat: {
                args_float_.emplace_back(std::stof(arg));
                break;
            }
            case ArgType::kSize: {
                args_size_.emplace_back(ToSize(arg));
                break;
            }
        }
        is_set_ = true;
        return (type_ != ArgType::kNone) ? 1 : 0;
    }
    int Add(void) {
        return Add(nullptr);
    }
    bool IsSet(void) {
        return is_set_;
    }
    std::string &GetStringOpt(void) { return args_str_.back(); }
    std::list<std::string> &GetStringOpts(void) { return args_str_; }
    int GetIntOpt(void) { return args_int_.back(); }
    std::list<int> &GetIntOpts(void) { return args_int_; }
    float GetFloatOpt(void) { return args_float_.back(); }
    std::list<float> &GetFloatOpts(void) { return args_float_; }
    size_t GetSizeOpt(void) { return args_size_.back(); }
    std::list<size_t> &GetSizeOpts(void) { return args_size_; }
};

class ArgMap {
protected:
    std::unordered_map<std::string, Arg> args_;

    void AddOpt(std::string opt, ArgType type = ArgType::kNone) {
        args_.emplace(opt, type);
    }

    void ArgIter(int argc, char **argv) {
        for(int i = 0; i < argc; ++i) {
            if(argv[i][0] != '-') {
                Arg &arg = args_[""];
                i += arg.Add(argv[i+1]);
                continue;
            }
            Arg &arg = args_[argv[i]];
            i += arg.Add(argv[i+1]);
        }
    }

    virtual void VerifyArgs(void) = 0;

public:
    ArgMap() = default;

    bool HasOpt(std::string opt) {
        return (args_.find(opt) == args_.end());
    }

    bool OptIsSet(std::string opt) {
        return args_[opt].IsSet();
    }

    std::string &GetStringOpt(std::string opt) { return args_[opt].GetStringOpt(); }
    std::list<std::string> &GetStringOpts(std::string opt) { return args_[opt].GetStringOpts(); }
    int GetIntOpt(std::string opt) { return args_[opt].GetIntOpt(); }
    std::list<int> &GetIntOpts(std::string opt) { args_[opt].GetIntOpts(); }
    float GetFloatOpt(std::string opt) { return args_[opt].GetFloatOpt(); }
    std::list<float> &GetFloatOpts(std::string opt) { return args_[opt].GetFloatOpts(); }
    size_t GetSizeOpt(std::string opt) { return args_[opt].GetSizeOpt(); }
    std::list<size_t> &GetSizeOpts(std::string opt) { return args_[opt].GetSizeOpts(); }

    virtual void Usage(void) = 0;
};

#endif