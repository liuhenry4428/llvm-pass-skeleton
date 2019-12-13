#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      errs() << F.getName() << '\n';
      for(auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
          arg->setName("argName");
          errs() << arg->getName() << "\n";
        }
      for (auto &bb : F) {
        for (auto &instruction : bb) {
          if (CallBase *callInst = dyn_cast<CallBase>(&instruction)) {
            if (Function *calledFunction = callInst->getCalledFunction()) {
              if (calledFunction->getName().startswith(F.getName())) {
                //errs() << "HENRY " << calledFunction->getName() << "!\n";
                for(auto arg = callInst->arg_begin(); arg != callInst->arg_end(); ++arg) {
                  auto* argValue =arg->get();
                  argValue->setName("henryName");
                  errs() << argValue->getName() << "\n";
                }
              }
            }
          }
        }
      }
      errs() << "I saw a function called " << F.getName() << "!\n";
      return false;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
