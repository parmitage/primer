def ProjectEuler1 = lambda (limit)
	def Inner = lambda (x)
		if (x < limit)
			if (x % 3 == 0 || x % 5 == 0)
				x + Inner(x + 1)
			else
				Inner(x + 1)
			end
		else
			0
		end
	end
	Inner(0)
end

Show(ProjectEuler1(1000))