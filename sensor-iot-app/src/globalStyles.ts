import { chakra, Box, Text } from "@chakra-ui/react";

export const Container = chakra(Box, {
  baseStyle: {
    backgroundColor: "#4B5261",
    minHeight: "100vh",
    display: "flex",
    flexDirection: "column",
    alignItems: "center",
    justifyContent: "center",
    fontSize: "calc(10px + 2vmin)",
    color: "#282c34",
},
});

export const Title = chakra(Text, {
    baseStyle: {
    fontSize: "64px",
    color: "white",
    fontFamily: "Monospace",
  },
});

export const SidebarBox = chakra(Box, {
  baseStyle: {
    left: 0,
    top: 0,
    h: "100%",
    w: "20%",
    bg: "#282c34",
    position: "fixed",
  },
});

export const Content = chakra(Box, {
  baseStyle: {
    bg: "#282c34",
    position: "sticky",
    float: "left",
    flex: "column",
    top: "0",
    color: "Gray.03",
    height: "100vh",
    backgroundColor: "White.02",
    display: { sm: "none", md: "block" },
    maxWidth: { sm: "none", md: "20em" },
    left: "0",
  },
});
