extern crate indextree;

use std::collections::HashMap;
use std::io;
use std::io::prelude::*;

use indextree::Arena;
use indextree::NodeEdge;
use indextree::NodeId;

fn main() {
    let stdin = io::stdin();

    let mut arena = Arena::new();

    let mut first_line = String::new();
    io::stdin().read_line(&mut first_line).expect("Failed to read line");
    let node_cnt: u32 = first_line.trim_right().parse().expect("Not an integer");

    let mut nodes: Vec<indextree::NodeId> = Vec::new();
    for index in 0..node_cnt {
        nodes.push(arena.new_node(index));
    }

    for line in stdin.lock().lines() {
        let edge: Vec<u32> =
            line.unwrap().split(" ").map(|x| x.parse().expect("Not an integer")).collect();
        nodes[edge[0] as usize].append(nodes[edge[1] as usize], &mut arena);
    }

    println!("digraph tree {{");
    for &parent_node in &nodes {
        for desc_node in parent_node.children(&arena) {
            println!("\t{} -> {}",
                     arena[parent_node].data,
                     arena[desc_node].data);
        }
    }
    println!("}}");

    let mut signatures: Vec<HashMap<u32, (u32, Vec<u32>)>> = Vec::with_capacity(node_cnt as usize);
    for _ in 0..node_cnt {
        signatures.push(HashMap::new());
    }

    for node_id in nodes[0].traverse(&arena) {
        if let NodeEdge::End(curr_id) = node_id {
            let prev_sibling_id = arena[curr_id].previous_sibling(); 
            let rightmost_child_id = arena[curr_id].last_child();

            if prev_sibling_id.is_none() && rightmost_child_id.is_none() {
                signatures[arena[curr_id].data as usize].insert(0, (0, Vec::new()));
                signatures[arena[curr_id].data as usize].insert(1, (1, Vec::new()));
            }
        }
    }
}
