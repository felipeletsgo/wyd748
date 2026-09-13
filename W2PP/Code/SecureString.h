#ifndef _SECURESTRING_H
#define _SECURESTRING_H

// Macro segura para copiar strings com garantia de null-termination
// Uso: STRCPY_SAFE(dest, src, sizeof(dest))
#define STRCPY_SAFE(dest, src, size) do { \
	strncpy(dest, src, size - 1); \
	dest[size - 1] = '\0'; \
} while(0)

// Macro segura para concatenar strings
#define STRNCAT_SAFE(dest, src, size) do { \
	size_t len = strlen(dest); \
	if (len < size - 1) { \
		strncpy(dest + len, src, size - len - 1); \
		dest[size - 1] = '\0'; \
	} \
} while(0)

// Macro segura para sprintf
#define SNPRINTF_SAFE(dest, size, fmt, ...) do { \
	snprintf(dest, size, fmt, ##__VA_ARGS__); \
	dest[size - 1] = '\0'; \
} while(0)

// Funcao inline para validar tamanho de nome
inline BOOL IsValidName(const char* name, int max_length)
{
	if (!name) return FALSE;
	int len = 0;
	while (len < max_length && name[len] != '\0')
	{
		// Verificar caracteres invalidos
		char c = name[len];
		if (c == '\'' || c == '"' || c == '\\' || c == ';' || c == '-')
			return FALSE;
		len++;
	}
	return len > 0 && len < max_length;
}

#endif // _SECURESTRING_H
