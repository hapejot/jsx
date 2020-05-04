
function CollatzTest(limit) {
	var me = this
	// this.limit = limit
	lengths = {}
	max = 0
	max_pos = 0

	// defines the step n -> n+1 for the collatz sequence
	function CollatzStep(n) {
		if (n % 2 == 0) { return n / 2 }
		else { return 3 * n + 1 }
	}

	// Print(new SeqSequence(cnt))
	// var el = Max(new SeqSequence(cnt))
	// print(el.start + " " + el.len)
	// this.length represents a caching mechanism in order to prevent the recalculation of an already known sequence lenth
	// this improves the performance by an order of a magnitude.
	function SeqLen(n) {
		if (lengths[n] === undefined) {
			var n1 = CollatzStep(n)
			var result = 0
			if (n1 > 1) result = 1 + SeqLen(n1)
			else result = 1
			lengths[n] = result

			if (result > max) {
				max = result
				max_pos = n
			}
			return result
		}
		else {
			return lengths[n]
		}
	}

	this.run = function (){
		for (var i = 1; i <= limit; i++)	SeqLen(i)
		return(	"max:" + max +
				"\npos:" + max_pos)
	}
}





var tst = new CollatzTest(100000000)

print(tst.run())
