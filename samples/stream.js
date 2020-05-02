

function CollatzStep(n) {
	if(n % 2 == 0)	{ return n / 2 }
	else 			{ return 3 * n + 1	}
}


function CollatzTest(limit){
	this.limit = limit
	function CollatzStep(n) {
		if(n % 2 == 0)	{ return n / 2 }
		else 			{ return 3 * n + 1	}
	}
}





// sequence of collatz numbers for a given start
// the start number is not part of the sequence so the length of it represents the number
// of steps.
function CollatzSequence(n){
	this.n = n	
	this.HasNext = function() { return (this.n > 1) }
	this.Next = function() {
		this.n = CollatzStep(this.n)
		return this.n
	}
}

// sequence of CollatzSequences for the range <num> .. 1
function SeqSequence(num) {
	this.n 			= num
	this.HasNext 	= function() { return this.n > 1 }
	this.Next 		= function() { return {start: this.n, len: Len(new CollatzSequence(this.n--))}}
}


function SeqLenSeq(num) {
	this.n 			= num
	this.len        = {}
	this.HasNext 	= function() { return this.n > 1 }
	this.Next 		= function() { return {start: this.n, len: Len(new CollatzSequence(this.n--))}}
}


// simple sequence just counting to zero,
// has nothing to do with the rest
function CountSequence(n) {
	this.n = n
	this.HasNext = function() { return this.n > 0 }
	this.Next = function() { return this.n--}
}


// Count the elements in the sequence
function Len(s) {
	var n = 0;
	while(s.HasNext()){
		s.Next()
		n++;
	}
	return n
}

// Max needs some details of the sequences content in order to 
// identify the the sequence number along with the value
// the sequences elements must be of the form {start:<obj>, len:<int>}
function Max(s) {
	var max = 0;		// value
	var start = 0;		// identification
	while(s.HasNext()){
		var el = s.Next()
		var x = el.len
		if(x>max) { 
			max = x
			start = el.start
		}
	}
	return {start: start, len: max}
}


function Print(s) {
	var n = 1
	while(s.HasNext()){
		var el = s.Next()
		var line = "" + n
		for(var i in el)
			line += " " + i + ": " + el[i]
		print(line)
		n++
	}
}

if(typeof print === 'undefined') print = console.log

var cnt = 100000000


var lengths = {}
var max = 0	
var max_pos = 0
// Print(new SeqSequence(cnt))
// var el = Max(new SeqSequence(cnt))
// print(el.start + " " + el.len)
function SeqLen(n) {
	if(lengths[n] === undefined){
		var n1 = CollatzStep(n)
		// print(n + "->" + n1)
		if( n1 > 1 ) 	result = 1 + SeqLen(n1)
		else 			result = 1
		lengths[n] = result
		
		if(result > max){
			max = result
			max_pos = n
		}
		
		return result

	}
	else{
		return lengths[n]
	}
}

for(var i=1;i<=cnt;i++)	SeqLen(i)

// for(var i in lengths){
// 	if(lengths[i] > max){
// 		max = lengths[i]
// 		max_pos = i
// 	}
// }
print("max:" + max)
print("pos:" + max_pos)

// Print(new CollatzSequence(9))
// print(Len(new CollatzSequence(13)))
