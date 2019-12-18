; ModuleID = 'fib.c'
source_filename = "fib.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

@.str = private unnamed_addr constant [9 x i8] c"Fib7: %i\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @fibbonacci(i32) {
BaseCaseStuff:
  %1 = icmp eq i32 0, %0
  br i1 %1, label %trueBlock, label %FalseBlock

trueBlock:                                        ; preds = %BaseCaseStuff
  ret i32 0

FalseBlock:                                       ; preds = %BaseCaseStuff
  %2 = icmp eq i32 1, %0
  br i1 %2, label %trueBlock1, label %IterativeBlock

trueBlock1:                                       ; preds = %FalseBlock
  ret i32 1

IterativeBlock:                                   ; preds = %FalseBlock
  %itPtr = alloca i32
  %i1Ptr = alloca i32
  %i2Ptr = alloca i32
  store i32 1, i32* %itPtr
  store i32 0, i32* %i1Ptr
  store i32 1, i32* %i2Ptr
  br label %WhileBody

WhileBody:                                        ; preds = %WhileBody, %IterativeBlock
  %itLoaded = load i32, i32* %itPtr
  %i1Loaded = load i32, i32* %i1Ptr
  %i2Loaded = load i32, i32* %i2Ptr
  %current = add i32 %i1Loaded, %i2Loaded
  store i32 %i2Loaded, i32* %i1Ptr
  store i32 %current, i32* %i2Ptr
  %increment = add i32 %itLoaded, 1
  store i32 %increment, i32* %itPtr
  %itLoadedAgain = load i32, i32* %itPtr
  %loopGuard = icmp slt i32 %itLoadedAgain, %0
  br i1 %loopGuard, label %WhileBody, label %returnBlock

returnBlock:                                      ; preds = %WhileBody
  ret i32 %current
}
; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 {
  %1 = call i32 @fibbonacci(i32 48)
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %1)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "darwin-stkchk-strong-link" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "darwin-stkchk-strong-link" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 10, i32 15]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"Apple clang version 11.0.0 (clang-1100.0.33.12)"}
