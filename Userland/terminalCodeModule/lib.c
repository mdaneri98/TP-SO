



int stringToInt(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Manejar el signo negativo si está presente
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    // Iterar sobre los caracteres de la cadena
    while (str[i] != '\0') {
        // Verificar si el carácter es un dígito válido
        if (str[i] >= '0' && str[i] <= '9') {
            // Actualizar el resultado multiplicándolo por 10 y sumándole el valor del dígito actual
            result = result * 10 + (str[i] - '0');
            i++;
        } else {
            // Carácter no válido, salir del bucle
            break;
        }
    }

    // Aplicar el signo al resultado
    result *= sign;

    return result;
}
