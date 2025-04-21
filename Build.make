building:
	zig c++ -o build/game.exe src/main.cpp -I"C:\raylib\include" -L"C:\raylib\lib" -target x86_64-windows -lraylib
	@echo Building done

run:building
	./build/game.exe

clean:
	del build/*.o build/*.exe build/*.pdb /s
	@echo Cleaning done
