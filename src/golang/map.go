package main
import "fmt"

func map1(){
	var a map[string]int = make(map[string]int)
	var b = make(map[string]int)
	c := make(map[string]int)

	fmt.Println("map1=========")

	a["kaka"] = 1

	fmt.Println(a)
	fmt.Println(b)
	fmt.Println(c)
}

func map2(){
	a := map[string]int{"Hello":10, "World":20}
	b := map[string]int{
		"Hello":10,
		"World":20,
	}

	fmt.Println("map2=========")
	fmt.Println(a)
	fmt.Println(b)
	fmt.Println(a["Hello"])
}

func map3(){
	a := map[string]int{"Hello":10, "World":20}
	b := a
	b["Sop"] = 30
	a["Uhosssss"] = 40
	fmt.Println("map3=========")
	fmt.Println(a)
	fmt.Println(b)
}

func map4(){
	fmt.Println("map4=========")
	solarSystem := make(map[string]float32)

	solarSystem["M"] = 87.11
	solarSystem["V"] = 224.12
	solarSystem["E"] = 3543.221

	{
		value, ok := solarSystem["E"]
		fmt.Println(value, ok)
	}
	{
		value, ok := solarSystem["D"]
		fmt.Println(value, ok)
	}
	{
		if value, ok := solarSystem["M"]; ok {
			fmt.Println(value)
		}
	}
	{
		if value, ok := solarSystem["G"]; ok {
			fmt.Println(value)
		}
	}
}

func map5(){
	fmt.Println("map5=========")
	a:= map[string]int{"Hello":10, "World":20}
	fmt.Println(a)
	delete(a, "Hello")
	fmt.Println(a)
}

func map6(){
	fmt.Println("map6=========")
	solarSystem := make(map[string]float32)

	solarSystem["M"] = 87.11
	solarSystem["V"] = 224.12
	solarSystem["E"] = 3543.221

	for key, value := range solarSystem {
		fmt.Println(key, value)
	}
}

func map7(){
	terrestrialPlanet := map[string]map[string]float32 {
		"Mercury" : map[string]float32 {
			"meanRadius": 234.2,
			"mass":       3212.22,
			"orbitalPeriod": 82.11,
		},
		"Venus" : map[string]float32 {
			"meanRadius": 111.2,
			"mass":       2222.22,
			"orbitalPeriod": 333.11,
		},
	}

	fmt.Println(terrestrialPlanet)
	fmt.Println(terrestrialPlanet["Mercury"]["mass"])
}

func main(){
	map1()
	map2()
	map3()
	map4()
	map5()
	map6()
	map7()
}
