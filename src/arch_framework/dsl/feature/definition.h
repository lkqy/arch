#include "arch_framework/core/ast/asm.h"
#include <fstream>

/*

struct InstanceProfile {
    xxxx
    yyyy
    std::vector<int64_t> fids; // 必须有的字段
};

*/

template <typename T>
class ModelFeature {
public:
    ModelFeature() {
    }
    bool init(const std::string& conf_file) {
        std::ifstream file(conf_file);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string raw_conf = buffer.str();
        std::cout << raw_conf << "\n";
        std::ostrstream exp_buffer;

        auto raw_ptr = raw_conf.c_str();

        size_t start = 0, end = 0;
        size_t line = 0;
        bool is_first = false;
        while (end < raw_conf.size()) {
            line += 1;
            // skip '#'
            size_t before_skip = end;
            while (raw_conf[before_skip] != '#' and raw_conf[before_skip] != '\n') {
                before_skip += 1;
            }
            end = before_skip;
            while (raw_conf[before_skip] == '#' and raw_conf[end] != '\n') {
                end += 1;
            }
            // 跳过纯碎空行
            if (start == before_skip) {
                end += 1;
                start = end;
                continue;
            }
            // 完整的一行配置[start, before_skip)
            // find _xxx
            size_t n_num = start;
            while (n_num < before_skip and isdigit(raw_conf[n_num]) == 0)
                ++n_num;

            size_t n_end = n_num;
            // find xxx =
            while (n_end < before_skip and raw_conf[n_end] != ' ' and raw_conf[n_end] != '=')
                ++n_end;

            std::string num(raw_ptr + n_num, n_end - n_num);

            if (n_num == n_end or n_end == before_skip) {
                // ToDo: error
                return false;
            }

            // find = xxx
            size_t n_exp = n_end;
            while (n_exp < before_skip and raw_conf[n_exp] != '=')
                ++n_exp;
            ++n_exp;

            std::string exp_str(raw_ptr + n_exp, before_skip - n_exp);

            // Append(fids, Mask(exp, 10, 5))
            if (is_first)
                exp_buffer << "; ";
            is_first = true;
            exp_buffer << "Append(fids, Mask(" << exp_str << ", 10, " << num << "))";

            end += 1;
            start = end;
        }
        feature_exp = std::string(exp_buffer.str(), exp_buffer.pcount());
        auto ctx = jit.get(feature_exp);
        if (ctx == nullptr) {
            // std::cout<<jit.error_log()<<"\n";
            return false;
        }
        return true;
    }
    bool extract(T& t) {
        auto ctx = jit.get(feature_exp);
        ctx->eval(t);
        return true;
    }

private:
    std::string feature_exp;
    Jit<T> jit;
};
