def ProjectEuler1 = lambda (limit)
	def Inner = lambda (x)
		if (x < limit)
			if (x % 3 == 0 || x % 5 == 0)
				return x + Inner(x + 1);
			else
				return Inner(x + 1);
			end
		else
			return 0;
		end
	end
	return Inner(0);
end

Show(ProjectEuler1(1000))