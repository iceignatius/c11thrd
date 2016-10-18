/******************************************************************************
 * Name      : Static Assert
 * Purpose   : To support a compile-time assert function.
 * Author    : 王文佑
 * Created   : 2014.01.20
 * Licence   : ZLib Licence
 * Reference : http://www.openfoundry.org/of/projects/2419
 ******************************************************************************/
#ifndef _IGN_STATIC_ASSERT_H_
#define _IGN_STATIC_ASSERT_H_

// 巨集中用來取得單一檔案內唯一識別名稱的功能，使用者一般不需理會及使用。
#define STATIC_ASSERT_GET_NAME_(name,line)  name##line
#define STATIC_ASSERT_GET_NAME(name,line)   STATIC_ASSERT_GET_NAME_(name,line)

// 公開的 static assert 巨集，分為 "一般通用" 及 "C89 函式內使用" 兩種模式。
#define STATIC_ASSERT(condition)        extern int STATIC_ASSERT_GET_NAME(static_assert_check,__LINE__)[(condition)?1:-1]
#define STATIC_ASSERT_CFUNC(condition)  {STATIC_ASSERT(condition)}

// 仿製 C++11 的 static assert 巨集
#if !defined(__cplusplus) || ( __cplusplus < 201103L )
    #ifndef static_assert
        #define static_assert(condition,message)    extern int STATIC_ASSERT_GET_NAME(static_assert_check,__LINE__)[(condition)?1:-1]
    #endif
#endif

#endif
