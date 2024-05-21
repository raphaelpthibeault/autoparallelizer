#ifndef __VISITORS_HPP
#define __VISITORS_HPP

#include <CBaseVisitor.h>
#include <CParser.h>
#include <semantic-concepts/concepts.hpp>

class GlobalVisitor : public CBaseVisitor {
private:
    Program &program;

public:
    GlobalVisitor(Program &program);
    ~GlobalVisitor();
    virtual antlrcpp::Any visitTranslationUnit(CParser::TranslationUnitContext *ctx) override;
};

#endif // __VISITORS_HPP
