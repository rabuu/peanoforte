if exists("b:current_syntax")
	finish
end

syn keyword peanoforteKeyword define theorem example base step induction by rev todo
syn keyword peanoforteOperator succ
syn keyword peanoforteZero 0
syn match peanoforteNumber "\<[1-9][0-9]*\>"
syn region peanoforteComment start=";" end="\n"

hi def link peanoforteKeyword Keyword
hi def link peanoforteOperator Operator
hi def link peanoforteZero Special
hi def link peanoforteNumber Number
hi def link peanoforteComment Comment

let b:current_syntax = "peanoforte"
