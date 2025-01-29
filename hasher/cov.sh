mkdir -p build/coverage
gcovr -r . --html-details build/coverage/index.html -e ../lib/
read -p "open in browser? (y/N): " open
if [ "$open" != "${open#[yY]}" ]; then
	xdg-open build/coverage/index.html;
fi
