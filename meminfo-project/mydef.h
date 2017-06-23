#ifndef MEMPROJECT_MYDEF_H
#define MEMPROJECT_MYDEF_H

#define CALL(var, fun, err, funName)\
    {\
        if((var = fun) == err){\
            perror(funName);\
            goto exit_bgn;\
        }\
    }

#define CALL_CHECK(fun, err, funName)\
    {\
        if(fun == err){\
            perror(funName);\
            goto exit_bgn;\
        }\
    }


#define CALL_CORRECT(fun, correct_val, funName)\
    {\
        if(fun != correct_val){\
            perror(funName);\
            goto exit_bgn;\
        }\
    }

#define EXIT_BGN\
    exit_bgn:{\

#define EXIT_END\
    }

#endif
