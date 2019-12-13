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
      for (auto &bb : F) {
        for (auto &instruction : bb) {
          if (CallBase *callInst = dyn_cast<CallBase>(&instruction)) {
            if (Function *calledFunction = callInst->getCalledFunction()) {
              if (calledFunction->getName().startswith(F.getName())) {
                errs() << "HENRY " << calledFunction->getName() << "!\n";
                for(auto arg = calledFunction->arg_begin(); arg != calledFunction->arg_end(); ++arg) {
                  if(auto* ci = dyn_cast<ConstantInt>(arg))
                    errs() << ci->getValue() << "\n";
                    errs() << *arg << "\n";
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
