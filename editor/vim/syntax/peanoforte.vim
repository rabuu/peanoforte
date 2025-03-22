if exists("b:current_syntax")
	finish
end

syn keyword peanoforteKeyword axiom theorem example base step induction
syn keyword peanoforteTodo todo
syn keyword peanoforteOperator succ
syn keyword peanoforteZero 0
syn match peanoforteNumber "\<[1-9][0-9]*\>"
syn match peanoforteApply "@\<[a-zA-Z-]*\>"
syn match peanoforteBackwardsApply "$\<[a-zA-Z-]*\>"
syn match peanoforteInductionApply "@induction"
syn match peanoforteBackwardsInductionApply "$induction"
syn region peanoforteComment start=";" end="\n"

hi def link peanoforteKeyword Keyword
hi def link peanoforteTodo Todo
hi def link peanoforteOperator Operator
hi def link peanoforteZero Special
hi def link peanoforteNumber Number
hi def link peanoforteApply Identifier
hi def link peanoforteBackwardsApply Identifier
hi def link peanoforteInductionApply Function
hi def link peanoforteBackwardsInductionApply Function
hi def link peanoforteComment Comment

let b:current_syntax = "peanoforte"
