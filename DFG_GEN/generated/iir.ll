
	%tmp.0 = load int* %x		; <int> [#uses=1]
	%tmp.1 = load int* %a1		; <int> [#uses=1]
	%tmp.2 = load int* %w1		; <int> [#uses=2]
	%tmp.3 = mul int %tmp.1, %tmp.2		; <int> [#uses=1]
	%tmp.4 = sub int %tmp.0, %tmp.3		; <int> [#uses=1]
	%tmp.6 = load int* %a2		; <int> [#uses=2]
	%tmp.7 = load int* %w2		; <int> [#uses=1]
	%tmp.8 = mul int %tmp.6, %tmp.7		; <int> [#uses=1]
	%tmp.9 = sub int %tmp.4, %tmp.8		; <int> [#uses=4]
	store int %tmp.9, int* %w
	%tmp.10 = load int* %b0		; <int> [#uses=1]
	%tmp.12 = mul int %tmp.10, %tmp.9		; <int> [#uses=1]
	%tmp.15 = add int %tmp.9, %tmp.6		; <int> [#uses=2]
	%tmp.16 = load int* %b1		; <int> [#uses=1]
	%tmp.17 = mul int %tmp.15, %tmp.16		; <int> [#uses=1]
	%tmp.19 = add int %tmp.17, %tmp.12		; <int> [#uses=1]
	%tmp.23 = load int* %b2		; <int> [#uses=1]
	%tmp.24 = mul int %tmp.23, 113		; <int> [#uses=1]
	%tmp.25 = mul int %tmp.24, %tmp.15		; <int> [#uses=1]
	%tmp.27 = add int %tmp.25, %tmp.19		; <int> [#uses=1]
	store int %tmp.27, int* %y
	store int %tmp.2, int* %w3
	%tmp.30 = sub int 0, %tmp.9		; <int> [#uses=1]
	store int %tmp.30, int* %w4


